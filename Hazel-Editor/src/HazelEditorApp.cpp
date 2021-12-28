#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Hazel
{
	class HazelEditor : public Application
	{
	public:
		HazelEditor(const ApplicationSpecification& specification, std::string_view scenePath)
			: Application(specification)
		{
			PushLayer(new EditorLayer(scenePath));
		}

		~HazelEditor() override = default;
	};

	Application* CreateApplication(int argc, char** argv)
	{
		std::string_view scenePath;
		if (argc > 1)
			scenePath = argv[0];

		ApplicationSpecification specification;
		specification.Name = "Hazel Editor";
		specification.WindowWidth = 1600;
		specification.WindowHeight = 900;
		specification.StartMaximized = true;
		specification.VSync = true;

		return new HazelEditor(specification, scenePath);
	}
}
