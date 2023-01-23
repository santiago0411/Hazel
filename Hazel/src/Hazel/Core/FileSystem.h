#pragma once

#include "Hazel/Core/Buffer.h"

#include <filesystem>

namespace Hazel
{
	using FilePath = std::filesystem::path;

	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const FilePath& filepath);
	};
}
