#pragma once

#include "Hazel/Core/Window.h"
#include "Hazel/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>


namespace Hazel
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowSpecification& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		uint32_t GetWidth() const override { return m_Data.Width; }
		uint32_t GetHeight() const override { return m_Data.Height; }

		void Maximize() override;
		void CenterWindow() override;

		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
		void SetResizable(bool resizable) const override;

		void* GetNativeWindow() const override { return m_Window; }
	private:
		void Init();
		void CreateGlfwWindow();
		void Shutdown();
	private:
		GLFWwindow* m_Window = nullptr;
		WindowSpecification m_Specification;
		Scope<GraphicsContext> m_Context;

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}


