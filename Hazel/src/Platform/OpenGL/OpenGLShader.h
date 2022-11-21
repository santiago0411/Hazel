#pragma once

#include "Hazel/Renderer/Shader.h"
#include <glm/glm.hpp>

using GLenum = uint32_t;

namespace Hazel
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const FilePath& filepath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader() override;

		void Bind() const override;
		void Unbind() const override;

		void SetInt(const std::string& name, const int32_t value) override;
		void SetIntArray(const std::string& name, const int32_t* values, const uint32_t count) override;
		void SetFloat(const std::string& name, const float value) override;
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;
		
		const std::string& GetName() const override { return m_Name; }

		void UploadUniformInt(const std::string& name, const int32_t value) const;
		void UploadUniformIntArray(const std::string& name, const int32_t* values, const uint32_t count) const;
		void UploadUniformFloat(const std::string& name, const float value) const;
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values) const;
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values) const;
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values) const;

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const;
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const;

	private:
		static std::string ReadFile(const FilePath& filepath);
		static std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);

		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);
	
	private:
		uint32_t m_RendererId;
		FilePath m_FilePath;
		std::string m_Name;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>> m_OpenGLSPIRV;
		
		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;
	};
}
