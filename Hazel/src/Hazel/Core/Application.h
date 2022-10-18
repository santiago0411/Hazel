#pragma once

#include "Hazel/Core/Base.h"
#include "Hazel/Core/LayerStack.h"
#include "Hazel/Core/Window.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/Event.h"

#include "Hazel/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Hazel
{
	struct ApplicationCommandLineArgs
	{
		int32_t Count = 0;
		char** Args = nullptr;

		const char* operator[](int32_t index) const
		{
			HZ_CORE_ASSERT(index < Count)
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Hazel Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		Window& GetWindow() const { return *m_Window; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		static Application& Get() { return *s_Instance; }

		void SubmitToMainThread(const std::function<void()>& function);
	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		void ExecuteMainThreadQueue();
	private:
		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;
	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
