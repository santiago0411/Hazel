project "Hazel-ScriptCore"
	kind "SharedLib"
	language "C#"

	targetdir ("%{wks.location}/Hazel-Editor/Resources/Scripts")
	objdir ("%{wks.location}/Hazel-Editor/Resources/Scripts/Intermediates")

	files
	{
		"Source/**.cs"
	}