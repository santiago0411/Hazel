#pragma once

#include <Hazel.h>

class Sandbox2D : public Hazel::Layer
{
public:
	Sandbox2D();
	~Sandbox2D() override = default;
	
	void OnAttach() override;
	void OnDetach() override;
	
	void OnUpdate(Hazel::Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;

private:
	Hazel::OrthographicCameraController m_CameraController;
	
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;
	Hazel::Ref<Hazel::Shader> m_FlatColorShader;
	Hazel::Ref<Hazel::Texture2D> m_CheckerBoardTexture;

	glm::vec4 m_SquareColor{ 0.2, 0.3f, 0.8f, 1.0f };
};

