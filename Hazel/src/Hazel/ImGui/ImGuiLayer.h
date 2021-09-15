#pragma once

#include "Hazel/Core/Layer.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/KeyEvent.h"
#include "Hazel/Events/MouseEvent.h"

namespace Hazel
{
	class ImGuiLayer final : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() override;

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& e) override;

		void Begin();
		void End();

		void SetBlockEvents(bool block) { m_BlockEvents = block; }

		static void SetDarkThemeColors();
	private:
		bool m_BlockEvents = true;
	};
}
