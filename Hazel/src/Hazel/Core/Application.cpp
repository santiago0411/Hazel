#include "hzpch.h"
#include "Hazel/Core/Application.h"

#include "Hazel/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

#include <ranges>

namespace Hazel
{
	Application* Application::s_Instance = nullptr;
	
	Application::Application(const std::string& name, ApplicationCommandLineArgs args)
		: m_CommandLineArgs(args)
	{
		HZ_PROFILE_FUNCTION();
		
		HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		
		m_Window = Window::Create(WindowProps{name, 1600, 900});
		m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(false);

		Renderer::Init();
		
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		HZ_PROFILE_FUNCTION();
		
		Renderer::Shutdown();
	}

	void Application::Run()
	{
		HZ_PROFILE_FUNCTION();
		
		while (m_Running)
		{
			HZ_PROFILE_SCOPE("RunLoop");
			
			const float time = (float)glfwGetTime();
			const Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

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

		for (const auto& layer : std::ranges::reverse_view(m_LayerStack))
		{
			if (e.Handled)
				break;
			layer->OnEvent(e);
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
}
