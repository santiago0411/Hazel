project "Hazel-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"
	namespace "Hazel"

	targetdir ("%{wks.location}/Hazel-Editor/Resources/Scripts")
	objdir ("%{wks.location}/Hazel-Editor/Resources/Scripts/Intermediates")

	files
	{
		"Source/**.cs",
		"Properties/**.cs"
	}

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"
