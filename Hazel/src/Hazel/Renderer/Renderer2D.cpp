#include "hzpch.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/VertexArray.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};
	
	struct Renderer2DData
	{
		static const uint32_t MAX_QUADS = 20000;
		static const uint32_t MAX_VERTICES = MAX_QUADS * 4;
		static const uint32_t MAX_INDICES = MAX_QUADS * 6;
		static const uint32_t MAX_TEXTURE_SLOTS = 32;
		
		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MAX_TEXTURE_SLOTS> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		glm::vec4 QuadVertexPositions[4];

		Renderer2D::Statistics Stats;
	};

	static Renderer2DData g_Data;
	
	void Renderer2D::Init()
	{
		HZ_PROFILE_FUNCTION();
		
		g_Data.QuadVertexArray = VertexArray::Create();

		g_Data.QuadVertexBuffer = VertexBuffer::Create(Renderer2DData::MAX_VERTICES * sizeof(QuadVertex));
		g_Data.QuadVertexBuffer->SetLayout(
		{
			{ ShaderDataType::Float3, "a_Position"		},
			{ ShaderDataType::Float4, "a_Color"			},
			{ ShaderDataType::Float2, "a_TexCoord"		},
			{ ShaderDataType::Float,  "a_TexIndex"		},
			{ ShaderDataType::Float,  "a_TilingFactor"	},
		});
		g_Data.QuadVertexArray->AddVertexBuffer(g_Data.QuadVertexBuffer);

		g_Data.QuadVertexBufferBase = new QuadVertex[Renderer2DData::MAX_VERTICES];

		auto quadIndices = new uint32_t[Renderer2DData::MAX_INDICES];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < Renderer2DData::MAX_INDICES; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;
			
			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		
		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, Renderer2DData::MAX_INDICES);
		g_Data.QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		g_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		g_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[Renderer2DData::MAX_TEXTURE_SLOTS]{};
		for (uint32_t i = 0; i < Renderer2DData::MAX_TEXTURE_SLOTS; i++)
			samplers[i] = i;
		
		g_Data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		g_Data.TextureShader->Bind();
		g_Data.TextureShader->SetIntArray("u_Textures", samplers, Renderer2DData::MAX_TEXTURE_SLOTS);

		g_Data.TextureSlots[0] = g_Data.WhiteTexture;

		g_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		g_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		g_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		g_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION();
		
		g_Data.TextureShader->Bind();
		g_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		g_Data.QuadIndexCount = 0;
		g_Data.QuadVertexBufferPtr = g_Data.QuadVertexBufferBase;

		g_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION();

		const auto dataSize = (uint32_t)((uint8_t*)g_Data.QuadVertexBufferPtr - (uint8_t*)g_Data.QuadVertexBufferBase);
		g_Data.QuadVertexBuffer->SetData(g_Data.QuadVertexBufferBase, dataSize);
		
		Flush();
	}

	void Renderer2D::Flush()
	{
		for (uint32_t i = 0; i < g_Data.TextureSlotIndex; i++)
			g_Data.TextureSlots[i]->Bind(i);
		
		RenderCommand::DrawIndexed(g_Data.QuadVertexArray, g_Data.QuadIndexCount);
		g_Data.Stats.DrawCalls++;
	}

	void Renderer2D::FlushAndReset()
	{
		EndScene();
		
		g_Data.QuadIndexCount = 0;
		g_Data.QuadVertexBufferPtr = g_Data.QuadVertexBufferBase;

		g_Data.TextureSlotIndex = 1;
	}
	
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		if (g_Data.QuadIndexCount >= Renderer2DData::MAX_INDICES)
			FlushAndReset();
		
		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[]{ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			const float textureIndex = 0.0f; // White texture index
			const float tilingFactor = 1.0f;
			
			g_Data.QuadVertexBufferPtr->Position = transform * g_Data.QuadVertexPositions[i];
			g_Data.QuadVertexBufferPtr->Color = color;
			g_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			g_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			g_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			g_Data.QuadVertexBufferPtr++;
		}

		g_Data.QuadIndexCount += 6;

		g_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor)
	{
		HZ_PROFILE_FUNCTION();

		if (g_Data.QuadIndexCount >= Renderer2DData::MAX_INDICES)
			FlushAndReset();
		
		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float textureIndex = 0.0f;
		
		for (uint32_t i = 1; i < g_Data.TextureSlotIndex; i++)
		{
			if (*g_Data.TextureSlots[i] == *texture)
			{
				textureIndex = (float)i;
				break;
			}
		}
		
		if (textureIndex == 0.0f)
		{
			if (g_Data.TextureSlotIndex >= Renderer2DData::MAX_TEXTURE_SLOTS)
				FlushAndReset();
			
			textureIndex = (float)g_Data.TextureSlotIndex;
			g_Data.TextureSlots[g_Data.TextureSlotIndex] = texture;
			g_Data.TextureSlotIndex++;
		}

		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[]{ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			g_Data.QuadVertexBufferPtr->Position = transform * g_Data.QuadVertexPositions[i];
			g_Data.QuadVertexBufferPtr->Color = color;
			g_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			g_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			g_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			g_Data.QuadVertexBufferPtr++;
		}

		g_Data.QuadIndexCount += 6;

		g_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		if (g_Data.QuadIndexCount >= Renderer2DData::MAX_INDICES)
			FlushAndReset();
		
		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[]{ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		
		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f ,1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			const float textureIndex = 0.0f; // White texture index
			const float tilingFactor = 1.0f;

			g_Data.QuadVertexBufferPtr->Position = transform * g_Data.QuadVertexPositions[i];
			g_Data.QuadVertexBufferPtr->Color = color;
			g_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			g_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			g_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			g_Data.QuadVertexBufferPtr++;
		}

		g_Data.QuadIndexCount += 6;

		g_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4 tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4 tintColor)
	{
		HZ_PROFILE_FUNCTION();

		if (g_Data.QuadIndexCount >= Renderer2DData::MAX_INDICES)
			FlushAndReset();
		
		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < g_Data.TextureSlotIndex; i++)
		{
			if (*g_Data.TextureSlots[i] == *texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (g_Data.TextureSlotIndex >= Renderer2DData::MAX_TEXTURE_SLOTS)
				FlushAndReset();
			
			textureIndex = (float)g_Data.TextureSlotIndex;
			g_Data.TextureSlots[g_Data.TextureSlotIndex] = texture;
			g_Data.TextureSlotIndex++;
		}

		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[]{ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		
		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f ,1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			g_Data.QuadVertexBufferPtr->Position = transform * g_Data.QuadVertexPositions[i];
			g_Data.QuadVertexBufferPtr->Color = color;
			g_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			g_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			g_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			g_Data.QuadVertexBufferPtr++;
		}


		g_Data.QuadIndexCount += 6;

		g_Data.Stats.QuadCount++;
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return g_Data.Stats;
	}

	void Renderer2D::ResetStats()
	{
		memset(&g_Data.Stats, 0, sizeof(Statistics));
	}
}
