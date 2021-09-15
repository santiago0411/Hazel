#include "hzpch.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "Hazel/Core/Timer.h"

namespace Hazel
{
	namespace Utils
	{
		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;

			HZ_CORE_ASSERT(false);
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
					return shaderc_glsl_vertex_shader;
				case GL_FRAGMENT_SHADER:
					return shaderc_glsl_fragment_shader;
			}

			HZ_CORE_ASSERT(false);
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
					return "GL_VERTEX_SHADER";
				case GL_FRAGMENT_SHADER:
					return "GL_FRAGMENT_SHADER";
			}

			HZ_CORE_ASSERT(false);
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			return "assets/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
					return ".cached_opengl.vert";
				case GL_FRAGMENT_SHADER:
					return ".cached_opengl.frag";
			}

			HZ_CORE_ASSERT(false);
			return nullptr;
		}

		static const char* GLShaderStageCachedVulkanFileExtension(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
					return ".cached_vulkan.vert";
				case GL_FRAGMENT_SHADER:
					return ".cached_vulkan.frag";
			}

			HZ_CORE_ASSERT(false);
			return nullptr;
		}
	}

	OpenGLShader::OpenGLShader(const std::string& filepath)
		: m_FilePath(filepath)
	{
		HZ_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);

		{
			Timer timer;
			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLBinaries();
			CreateProgram();
			HZ_CORE_TRACE("Shader creation took {0} ms", timer.ElapsedMillis());
		}

		// Extract name from filepath
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		HZ_PROFILE_FUNCTION();
		
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;

		{
			Timer timer;
			CompileOrGetVulkanBinaries(sources);
			CompileOrGetOpenGLBinaries();
			CreateProgram();
			HZ_CORE_TRACE("Shader creation took {0} ms", timer.ElapsedMillis());
		}
	}

	OpenGLShader::~OpenGLShader()
	{
		HZ_PROFILE_FUNCTION();
		
		glDeleteProgram(m_RendererId);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		HZ_PROFILE_FUNCTION();
		
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();

			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				HZ_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			HZ_CORE_ERROR("Could not open shader file '{0}'", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		HZ_PROFILE_FUNCTION();
		
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		const size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); // Start of shader type declaration line
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); // End of shader type declaration line
			HZ_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; // Start of shader type name (after #type keyword)
			std::string type = source.substr(begin, eol - begin);
			HZ_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); // Start of shader code after shader type declaration line
			HZ_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); // Start of next shader type declaration line
			
			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();

		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				size_t size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					HZ_CORE_ERROR(result.GetErrorMessage());
					HZ_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector(result.cbegin(), result.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		// TODO move elsewhere
		for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		shaderData.clear();
		m_OpenGLSourceCode.clear();

		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				size_t size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_OpenGLSourceCode[stage] = glslCompiler.compile();
				auto& source = m_OpenGLSourceCode[stage];

				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str());
				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					HZ_CORE_ERROR(result.GetErrorMessage());
					HZ_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector(result.cbegin(), result.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIds;

		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderId = shaderIds.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderId, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderId, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderId);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			HZ_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIds)
				glDeleteShader(id);
		}

		for (auto id : shaderIds)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererId = program;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		HZ_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_FilePath);
		HZ_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		HZ_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		HZ_CORE_TRACE("Uniform buffers:");
		for (const auto& [id, typeId, baseTypeId, name] : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(baseTypeId);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(id, spv::DecorationBinding);
			size_t memberCount = bufferType.member_types.size();

			HZ_CORE_TRACE("  {0}", name);
			HZ_CORE_TRACE("    Size = {0}", bufferSize);
			HZ_CORE_TRACE("    Binding = {0}", binding);
			HZ_CORE_TRACE("    Members = {0}", memberCount);
		}
	}

	void OpenGLShader::Bind() const
	{
		HZ_PROFILE_FUNCTION();

		glUseProgram(m_RendererId);
	}

	void OpenGLShader::Unbind() const
	{
		HZ_PROFILE_FUNCTION();
		
		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, const int32_t value)
	{
		HZ_PROFILE_FUNCTION();
		
		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, const int32_t* values, const uint32_t count)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, const float value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		HZ_PROFILE_FUNCTION();
		
		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		HZ_PROFILE_FUNCTION();
		
		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		HZ_PROFILE_FUNCTION();
		
		UploadUniformMat4(name, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, const int32_t value) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, const int32_t* values, const uint32_t count) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, const float value) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& values) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform2f(location, values.x, values.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& values) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform3f(location, values.x, values.y, values.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& values) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
	
	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}
