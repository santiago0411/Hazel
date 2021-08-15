#pragma once

#include "Hazel/Renderer/Buffer.h"

namespace Hazel
{
    class OpenGLVertexBuffer : public VertexBuffer
    {
    public:
        OpenGLVertexBuffer(float* vertices, uint32_t size);
        ~OpenGLVertexBuffer() override;

        void Bind() const override;
        void Unbind() const override;

        void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
        const BufferLayout& GetLayout() override { return m_Layout; }

    private:
        uint32_t m_RendererId;
        BufferLayout m_Layout;
    };

    class OpenGLIndexBuffer : public IndexBuffer
    {
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
        ~OpenGLIndexBuffer() override;

        void Bind() const override;
        void Unbind() const override;

        uint32_t GetCount() const override { return m_Count; }
    private:
        uint32_t m_RendererId;
        uint32_t m_Count;
    };
}
