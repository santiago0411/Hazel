#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Hazel/Asset/TextureImporter.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION();
	
	m_CheckerBoardTexture = Hazel::TextureImporter::LoadTexture2D("assets/textures/Checkerboard.png");
	m_SpriteSheet = Hazel::TextureImporter::LoadTexture2D("assets/game/textures/RPGpack_sheet_2X.png");

	m_TextureStairs = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 6 }, { 128, 128 });
	m_TextureBarrel = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 8, 2 }, { 128, 128 });
	m_TextureGrass = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1, 11 }, { 128, 128 });
	m_TextureTree = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });
}

void Sandbox2D::OnDetach()
{
	HZ_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION();

	m_CameraController.OnUpdate(ts);

	Hazel::Renderer2D::ResetStats();
	{
		HZ_PROFILE_SCOPE("Renderer Prep");
		Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Hazel::RenderCommand::Clear();
	}

	Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());

	for (int32_t y = 0; y < 20; y++)
	{
		for (int32_t x = 0; x < 20; x++)
		{
			Hazel::Renderer2D::DrawQuad({ x - 10, y - 10, -0.1f }, { 1.0f, 1.0f }, m_TextureGrass);
		}
	}
	
	//Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, m_TextureStairs);
	Hazel::Renderer2D::DrawQuad({ 1.0f, 0.0f, 0.1f }, { 1.0f, 1.0f }, m_TextureBarrel);
	Hazel::Renderer2D::DrawQuad({ 2.0f, 3.5f, 0.1f }, { 1.0f, 2.0f }, m_TextureTree);
	Hazel::Renderer2D::DrawQuad({ 5.0f, 1.0f, 0.1f }, { 1.0f, 2.0f }, m_TextureTree);
	Hazel::Renderer2D::DrawQuad({ -9.5f, -5.0f, 0.1f }, { 1.0f, 2.0f }, m_TextureTree);
	Hazel::Renderer2D::DrawQuad({ -8.0f, 3.0f, 0.1f }, { 1.0f, 2.0f }, m_TextureTree);
	Hazel::Renderer2D::DrawQuad({ -3.3f, 6.0f, 0.1f }, { 1.0f, 2.0f }, m_TextureTree);
	Hazel::Renderer2D::DrawQuad({ 2.7f, 7.0f, 0.1f }, { 1.0f, 2.0f }, m_TextureTree);
	Hazel::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	HZ_PROFILE_FUNCTION();

	ImGui::Begin("Renderer2D Stats");
	const auto stats = Hazel::Renderer2D::GetStats();
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Triangles: %d", stats.QuadCount * 2);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);
}
