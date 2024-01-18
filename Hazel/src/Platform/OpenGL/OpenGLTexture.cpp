#include "hzpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Hazel
{
	namespace Utils
	{
		static GLenum HazelImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGB8:  return GL_RGB;
				case ImageFormat::RGBA8: return GL_RGBA;
			}

			HZ_CORE_ASSERT(false)
			return 0;
		}

		static GLenum HazelImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGB8:  return GL_RGB8;
				case ImageFormat::RGBA8: return GL_RGBA8;
			}

			HZ_CORE_ASSERT(false)
			return 0;
		}

	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification, Buffer data)
		: m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
	{
		HZ_PROFILE_FUNCTION();

		m_InternalFormat = Utils::HazelImageFormatToGLInternalFormat(m_Specification.Format);
		m_DataFormat = Utils::HazelImageFormatToGLDataFormat(m_Specification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
		glTextureStorage2D(m_RendererId, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (data)
			SetData(data);
	} 

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		HZ_PROFILE_FUNCTION();
		
		glDeleteTextures(1, &m_RendererId);
	}

	void OpenGLTexture2D::ChangeSize(uint32_t newWidth, uint32_t newHeight)
	{
		uint32_t newTextureId;
		glCreateTextures(GL_TEXTURE_2D, 1, &newTextureId);
		glTextureStorage2D(newTextureId, 1, m_InternalFormat, newWidth, newHeight);

		glTextureParameteri(newTextureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(newTextureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(newTextureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(newTextureId, GL_TEXTURE_WRAP_T, GL_REPEAT);

		uint32_t framebufferRendererIds[2] = { 0 };
		glGenFramebuffers(2, framebufferRendererIds);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferRendererIds[0]);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RendererId, 0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferRendererIds[1]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newTextureId, 0);

		glBlitFramebuffer(0, 0, m_Width, m_Height, 0, 0, newWidth, newHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glDeleteTextures(1, &m_RendererId);
		glDeleteFramebuffers(2, framebufferRendererIds);

		m_RendererId = newTextureId;
		m_Width = newWidth;
		m_Height = newHeight;
	}

	void OpenGLTexture2D::SetData(Buffer data)
	{
		HZ_PROFILE_FUNCTION();
		
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		HZ_CORE_ASSERT(data.Size == (uint64_t)(m_Width * m_Height * bpp), "Data must be entire texture!")
		glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.Data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		HZ_PROFILE_FUNCTION();
		
		glBindTextureUnit(slot, m_RendererId);
	}
}
