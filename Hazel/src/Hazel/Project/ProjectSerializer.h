#pragma once

#include "Project.h"

namespace Hazel
{
	class ProjectSerializer
	{
	public:
		ProjectSerializer(Ref<Project> project);

		bool Serialize(const FilePath& path);
		bool Deserialize(const FilePath& path);

	private:
		Ref<Project> m_Project;
	};
}
