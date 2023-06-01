#pragma once

#include "Hazel/Renderer/Texture.h"

#include <glad/glad.h>

namespace Hazel
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureSpecification& specification, Buffer data);
		~OpenGLTexture2D() override;

		const TextureSpecification& GetSpecification() const override { return m_Specification; }

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }
		uint32_t GetRendererId() const override { return m_RendererId; }

		void SetData(Buffer buffer) override;
		
		void Bind(uint32_t slot = 0) const override;

		bool IsLoaded() const override { return m_IsLoaded; }

		bool operator==(const Texture& other) const override { return m_RendererId == ((const OpenGLTexture2D&)other).m_RendererId; }

	private:
		TextureSpecification m_Specification;

		bool m_IsLoaded = false;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererId;
		GLenum m_InternalFormat, m_DataFormat;
	};
}
