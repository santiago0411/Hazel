#include "hzpch.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/UniformBuffer.h"
#include "Hazel/Renderer/VertexArray.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Hazel
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		uint32_t TexIndex;
		float TilingFactor;

		// Editor-only
		int32_t EntityId;
	};

	struct CircleVertex
	{
		glm::vec3 WorldPosition;
		float Thickness;
		glm::vec2 LocalPosition;
		glm::vec4 Color;

		// Editor-only
		int32_t EntityId;
	};
	
	struct Renderer2DData
	{
		static constexpr uint32_t MAX_QUADS = 20000;
		static constexpr uint32_t MAX_VERTICES = MAX_QUADS * 4;
		static constexpr uint32_t MAX_INDICES = MAX_QUADS * 6;
		static constexpr uint32_t MAX_TEXTURE_SLOTS = 32;

		// Quads
		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		// Circles
		Ref<VertexArray> CircleVertexArray;
		Ref<VertexBuffer> CircleVertexBuffer;
		Ref<Shader> CircleShader;

		uint32_t CircleIndexCount = 0;
		CircleVertex* CircleVertexBufferBase = nullptr;
		CircleVertex* CircleVertexBufferPtr = nullptr;

		Ref<Texture2D> TextureSlots[MAX_TEXTURE_SLOTS];
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		glm::vec4 QuadVertexPositions[4]
		{
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};

		Renderer2D::Statistics Stats;

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
	};

	static Renderer2DData* s_Data;
	
	void Renderer2D::Init()
	{
		HZ_PROFILE_FUNCTION();

		s_Data = new Renderer2DData;

		// Quads
		{
			s_Data->QuadVertexArray = VertexArray::Create();

			s_Data->QuadVertexBuffer = VertexBuffer::Create(Renderer2DData::MAX_VERTICES * sizeof(QuadVertex));
			s_Data->QuadVertexBuffer->SetLayout(
				{
					{ ShaderDataType::Float3,	"a_Position"		},
					{ ShaderDataType::Float4,	"a_Color"			},
					{ ShaderDataType::Float2,	"a_TexCoord"		},
					{ ShaderDataType::Int,		"a_TexIndex"		},
					{ ShaderDataType::Float,	"a_TilingFactor"	},
					{ ShaderDataType::Int,		"a_EntityId"		},
				});
			s_Data->QuadVertexArray->AddVertexBuffer(s_Data->QuadVertexBuffer);

			s_Data->QuadVertexBufferBase = new QuadVertex[Renderer2DData::MAX_VERTICES];

			const auto quadIndices = new uint32_t[Renderer2DData::MAX_INDICES];

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

			const auto quadIB = IndexBuffer::Create(quadIndices, Renderer2DData::MAX_INDICES);
			s_Data->QuadVertexArray->SetIndexBuffer(quadIB);
			delete[] quadIndices;
		}

		// Circles
		{
			s_Data->CircleVertexArray = VertexArray::Create();

			s_Data->CircleVertexBuffer = VertexBuffer::Create(Renderer2DData::MAX_VERTICES * sizeof(CircleVertex));
			s_Data->CircleVertexBuffer->SetLayout(
				{
					{ ShaderDataType::Float3,	"a_WorldPosition"	},
					{ ShaderDataType::Float,	"a_Thickness"		},
					{ ShaderDataType::Float2,	"a_LocalPosition"	},
					{ ShaderDataType::Float4,	"a_Color"			},
					{ ShaderDataType::Int,		"a_EntityId"		}
				});
			s_Data->CircleVertexArray->AddVertexBuffer(s_Data->CircleVertexBuffer);

			s_Data->CircleVertexBufferBase = new CircleVertex[Renderer2DData::MAX_VERTICES];
			s_Data->CircleVertexArray->SetIndexBuffer(s_Data->QuadVertexArray->GetIndexBuffer());
		}

		s_Data->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		
		s_Data->TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data->CircleShader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");

		s_Data->CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);

		s_Data->TextureSlots[0] = s_Data->WhiteTexture;
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

		delete[] s_Data->QuadVertexBufferBase;
		delete[] s_Data->CircleVertexBufferBase;
		delete s_Data;
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& cameraTransform)
	{
		HZ_PROFILE_FUNCTION();

		s_Data->CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(cameraTransform);
		s_Data->CameraUniformBuffer->SetData(&s_Data->CameraBuffer, sizeof(Renderer2DData::CameraData));

		StartBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		s_Data->CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data->CameraUniformBuffer->SetData(&s_Data->CameraBuffer, sizeof(Renderer2DData::CameraData));

		StartBatch();
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION();
		
		s_Data->CameraBuffer.ViewProjection = camera.GetViewProjectionMatrix();
		s_Data->CameraUniformBuffer->SetData(&s_Data->CameraBuffer, sizeof(Renderer2DData::CameraData));

		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION();
		
		Flush();
	}

	void Renderer2D::StartBatch()
	{
		s_Data->QuadIndexCount = 0;
		s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase;

		s_Data->CircleIndexCount = 0;
		s_Data->CircleVertexBufferPtr = s_Data->CircleVertexBufferBase;

		s_Data->TextureSlotIndex = 1;
	}

	void Renderer2D::Flush()
	{
		if (s_Data->QuadIndexCount)
		{
			const auto dataSize = (uint32_t)((uint8_t*)s_Data->QuadVertexBufferPtr - (uint8_t*)s_Data->QuadVertexBufferBase);
			s_Data->QuadVertexBuffer->SetData(s_Data->QuadVertexBufferBase, dataSize);

			for (uint32_t i = 0; i < s_Data->TextureSlotIndex; i++)
				s_Data->TextureSlots[i]->Bind(i);

			s_Data->TextureShader->Bind();
			RenderCommand::DrawIndexed(s_Data->QuadVertexArray, s_Data->QuadIndexCount);
			s_Data->Stats.DrawCalls++;
		}

		if (s_Data->CircleIndexCount)
		{
			const auto dataSize = (uint32_t)((uint8_t*)s_Data->CircleVertexBufferPtr - (uint8_t*)s_Data->CircleVertexBufferBase);
			s_Data->CircleVertexBuffer->SetData(s_Data->CircleVertexBufferBase, dataSize);

			s_Data->CircleShader->Bind();
			RenderCommand::DrawIndexed(s_Data->CircleVertexArray, s_Data->CircleIndexCount);
			s_Data->Stats.DrawCalls++;
		}
	}

	void Renderer2D::NextBatch()
	{
		Flush();
		StartBatch();
	}
	
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4 tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4 tintColor)
	{
		HZ_PROFILE_FUNCTION();

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4 tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4 tintColor)
	{
		HZ_PROFILE_FUNCTION();

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f ,1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4 tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4 tintColor)
	{
		HZ_PROFILE_FUNCTION();

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f ,1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4 tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4 tintColor)
	{
		HZ_PROFILE_FUNCTION();

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f ,1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		if (s_Data->QuadIndexCount >= Renderer2DData::MAX_INDICES)
			NextBatch();

		constexpr glm::vec2 textureCoords[]{ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		constexpr uint32_t textureIndex = 0; // White texture index
		constexpr float tilingFactor = 1.0f;

		LoadQuadVertexData(transform, color, textureCoords, textureIndex, tilingFactor);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4 tintColor)
	{
		HZ_PROFILE_FUNCTION();

		if (s_Data->QuadIndexCount >= Renderer2DData::MAX_INDICES)
			NextBatch();

		const uint32_t textureIndex = FindTextureIndex(texture);
		constexpr glm::vec2 textureCoords[]{ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		LoadQuadVertexData(transform, tintColor, textureCoords, textureIndex, tilingFactor);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4 tintColor)
	{
		HZ_PROFILE_FUNCTION();

		if (s_Data->QuadIndexCount >= Renderer2DData::MAX_INDICES)
			NextBatch();

		const uint32_t textureIndex = FindTextureIndex(subTexture->GetTexture());
		const glm::vec2* textureCoords = subTexture->GetTexCoords();

		LoadQuadVertexData(transform, tintColor, textureCoords, textureIndex, tilingFactor);
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, float thickness, const glm::vec4& color, int32_t entityId)
	{
		constexpr size_t circleVertexCount = 4;

		for (size_t i = 0; i < circleVertexCount; i++)
		{
			s_Data->CircleVertexBufferPtr->WorldPosition = transform * s_Data->QuadVertexPositions[i];
			s_Data->CircleVertexBufferPtr->Thickness = thickness;
			s_Data->CircleVertexBufferPtr->LocalPosition = s_Data->QuadVertexPositions[i] * 2.0f;
			s_Data->CircleVertexBufferPtr->Color = color;
			s_Data->CircleVertexBufferPtr->EntityId = entityId;
			s_Data->CircleVertexBufferPtr++;
		}

		s_Data->CircleIndexCount += 6;

		s_Data->Stats.CircleCount++;
	}

	void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int32_t entityId)
	{
		HZ_PROFILE_FUNCTION();

		if (s_Data->QuadIndexCount >= Renderer2DData::MAX_INDICES)
			NextBatch();

		constexpr glm::vec2 textureCoords[]{ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		const uint32_t textureIndex = src.Texture ? FindTextureIndex(src.Texture) : 0;

		LoadQuadVertexData(transform, src.Color, textureCoords, textureIndex, src.TilingFactor, entityId);
	}

	uint32_t Renderer2D::FindTextureIndex(const Ref<Texture2D>& texture)
	{
		uint32_t textureIndex = 0;

		for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
		{
			if (*s_Data->TextureSlots[i] == *texture)
			{
				textureIndex = i;
				break;
			}
		}

		if (textureIndex == 0)
		{
			if (s_Data->TextureSlotIndex >= Renderer2DData::MAX_TEXTURE_SLOTS)
				NextBatch();

			textureIndex = s_Data->TextureSlotIndex;
			s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
			s_Data->TextureSlotIndex++;
		}

		return textureIndex;
	}

	void Renderer2D::LoadQuadVertexData(const glm::mat4& transform, const glm::vec4& color, glm::vec2 const* textureCoords, uint32_t textureIndex, float tilingFactor, int32_t entityId)
	{
		constexpr size_t quadVertexCount = 4;
		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data->QuadVertexBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->QuadVertexBufferPtr->Color = color;
			s_Data->QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data->QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data->QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data->QuadVertexBufferPtr->EntityId = entityId;
			s_Data->QuadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;

		s_Data->Stats.QuadCount++;
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data->Stats;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data->Stats, 0, sizeof(Statistics));
	}
}
