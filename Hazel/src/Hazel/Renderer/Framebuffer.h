#pragma once

namespace Hazel
{
	enum class FramebufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format) {}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		FramebufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};
	
	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;
		
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int32_t ReadPixel(uint32_t attachmentIndex, int32_t x, int32_t y) = 0;

		virtual void ClearAttachment(uint32_t attachmentIndex, int32_t value) = 0;

		virtual uint32_t GetColorAttachmentRendererId(uint32_t index = 0) const = 0;
		
		virtual const FramebufferSpecification& GetSpecification() const = 0;
		
		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}
