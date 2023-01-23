#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Hazel
{
	class HazelEditor : public Application
	{
	public:
		HazelEditor(const ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new EditorLayer());
		}

		~HazelEditor() override = default;
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Hazel Editor";
		spec.CommandLineArgs = args;

		spec.ScriptEngineConfig.CoreAssemblyPath = "Resources/Scripts/Hazel-ScriptCore.dll";
		spec.ScriptEngineConfig.EnableDebugging = true;

		return new HazelEditor(spec);
	}
}
