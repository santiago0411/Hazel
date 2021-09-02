#pragma once

#include <Hazel.h>

namespace Hazel
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(Timestep ts) override;
		void OnImGuiRender() override;
		void OnEvent(Event& e) override;

	private:
		OrthographicCameraController m_CameraController;

		Ref<Texture2D> m_CheckerBoardTexture;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		entt::entity m_SquareEntity;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = glm::vec2(0.0f);
	};
}
