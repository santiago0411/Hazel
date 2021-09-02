#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include <imgui/imgui.h>

#include "Sandbox2D.h"

class HazelEditor : public Hazel::Application
{
public:
	HazelEditor()
		: Application("Sandbox")
	{
		PushLayer(new Sandbox2D());
	}

	~HazelEditor() override = default;
};

Hazel::Application* Hazel::CreateApplication()
{
	return new HazelEditor();
}