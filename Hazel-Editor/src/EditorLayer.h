#pragma once

#include <Hazel.h>
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

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
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void OnOverlayRender();

		void NewProject();
		bool OpenProject();
		void OpenProject(const FilePath& path);
		void SaveProject();

		void NewScene();
		void OpenScene();
		void OpenScene(const FilePath& path);
		void SaveScene();
		void SaveSceneAs();
		void SerializeScene(const FilePath& path) const;

		int32_t GetMouseOverPixelData() const;

		void OnScenePlay();
		void OnSceneSimulate();
		void OnSceneStop();
		void OnScenePause();


		void DuplicateSelectedEntity();

		void UI_ToolBar();
	private:
		Ref<Framebuffer> m_Framebuffer;
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;
		FilePath m_ActiveScenePath;
		EditorCamera m_EditorCamera;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = glm::vec2(0.0f);
		glm::vec2 m_ViewportBounds[2];

		int32_t m_GizmoType = -1;

		bool m_ShowPhysicsColliders = false;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		Scope<ContentBrowserPanel> m_ContentBrowserPanel;

		// Editor resources
		enum class SceneState
		{
			Edit = 0, Play = 1, Simulate = 2
		};

		SceneState m_SceneState = SceneState::Edit;

		Ref<Texture2D> m_IconPlay, m_IconPause, m_IconStep, m_IconStop, m_IconSimulate;
	};
}
