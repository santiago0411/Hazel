#include "hzpch.h"
#include "Platform/Windows/WindowsWindow.h"

#include "Hazel/Core/Input.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/KeyEvent.h"
#include "Hazel/Events/MouseEvent.h"

#include "Hazel/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLContext.h"

namespace Hazel
{
	static uint8_t g_GLFWWindowCount = 0;

	static void GlfwErrorCallback(int32_t error, const char* description)
	{
		HZ_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}
	
	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		HZ_PROFILE_FUNCTION();

		WindowsWindow::Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		HZ_PROFILE_FUNCTION();

		WindowsWindow::Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		HZ_PROFILE_FUNCTION();
		
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		HZ_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (g_GLFWWindowCount == 0)
		{
			HZ_PROFILE_SCOPE("glfwInit");
			int32_t success = glfwInit();
			HZ_CORE_ASSERT(success, "Could not initialize GLFW!")
			glfwSetErrorCallback(GlfwErrorCallback);
		}

		{
			HZ_PROFILE_SCOPE("glfwCreateWindow");
		#if defined(HZ_DEBUG)
			if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		#endif

			CreateGlfwWindow();
		}
			
		m_Context = GraphicsContext::Create(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int32_t width, int32_t height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int32_t key, int32_t scanCode, int32_t action, int32_t modes)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, false);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, uint32_t keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});
		
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int32_t button, int32_t action, int32_t modes)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffSet)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffSet);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});

		glfwSetDropCallback(m_Window, [](GLFWwindow* window, int pathCount, const char* paths[])
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			std::vector<FilePath> filepaths(pathCount);
			for (int i = 0; i < pathCount; i++)
				filepaths[i] = paths[i];

			WindowDropEvent event(std::move(filepaths));
			data.EventCallback(event);
		});
	}

	void WindowsWindow::CreateGlfwWindow()
	{
		int32_t count;
		int32_t monitorX, monitorY;

		// Get the monitor viewport and the video mode spec
		GLFWmonitor** monitors = glfwGetMonitors(&count);
		glfwGetMonitorPos(monitors[0], &monitorX, &monitorY);
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[0]);

		// Set the visibility hint to to false for the window creation
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		// Create the glfw window
		m_Window = glfwCreateWindow((int32_t)m_Data.Width, (int32_t)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);

		// Set its position to be centered based on the width and height of the monitor
		glfwSetWindowPos(m_Window, 
			monitorX + (int32_t)((videoMode->width - m_Data.Width) / 2), 
			monitorY + (int32_t)((videoMode->height - m_Data.Height )/ 2));


		// Finally show the window and increase the window count
		glfwShowWindow(m_Window);
		g_GLFWWindowCount++;
	}

	void WindowsWindow::Shutdown()
	{
		HZ_PROFILE_FUNCTION();
		
		glfwDestroyWindow(m_Window);
		if (--g_GLFWWindowCount == 0)
		{
			HZ_CORE_INFO("Terminating GLFW");
			glfwTerminate();
		}
	}

	void WindowsWindow::OnUpdate()
	{
		HZ_PROFILE_FUNCTION();
		
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		HZ_PROFILE_FUNCTION();
		
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
}
