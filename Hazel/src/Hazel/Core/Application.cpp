#include "hzpch.h"
#include "Hazel/Core/Application.h"

#include "Hazel/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace Hazel
{
	Application* Application::s_Instance = nullptr;
	
	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		HZ_PROFILE_FUNCTION();
		
		HZ_CORE_ASSERT(!s_Instance, "Application already exists!")
		s_Instance = this;

		// Set working directory
		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		m_Window = Window::Create(WindowProps{m_Specification.Name, 1920, 1080});
		m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(false);

		Renderer::Init();
		ScriptEngine::Init();
		
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		HZ_PROFILE_FUNCTION();

		ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}

	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock lock(m_MainThreadQueueMutex);
		m_MainThreadQueue.emplace_back(function);
	}

	void Application::Run()
	{
		HZ_PROFILE_FUNCTION();
		
		while (m_Running)
		{
			HZ_PROFILE_SCOPE("RunLoop");
			
			const auto time = (float)glfwGetTime();
			const Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			ExecuteMainThreadQueue();

			if (!m_Minimized)
			{
				{
					HZ_PROFILE_SCOPE("LayerStack OnUpdate");
					
					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(timestep);
				}
				
				m_ImGuiLayer->Begin();
				{
					HZ_PROFILE_SCOPE("LayerStack OnImGuiRender");
					
					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();
				}
				m_ImGuiLayer->End();
			}

			m_Window->OnUpdate();
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		HZ_PROFILE_FUNCTION();
		
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		HZ_PROFILE_FUNCTION();
		
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnEvent(Event& e)
	{
		HZ_PROFILE_FUNCTION();
		
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		HZ_PROFILE_FUNCTION();
		
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}
		
		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		
		return false;
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock lock(m_MainThreadQueueMutex);

		for (auto& func : m_MainThreadQueue)
			func();

		m_MainThreadQueue.clear();
	}
}
