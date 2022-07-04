#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include <imgui/imgui.h>

#include "Sandbox2D.h"

class HazelEditor : public Hazel::Application
{
public:
	HazelEditor(const Hazel::ApplicationSpecification& specification)
		: Application(specification)
	{
		PushLayer(new Sandbox2D());
	}

	~HazelEditor() override = default;
};

Hazel::Application* Hazel::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.WorkingDirectory = "../Hazel-Editor";
	spec.CommandLineArgs = args;

	return new HazelEditor(spec);
}