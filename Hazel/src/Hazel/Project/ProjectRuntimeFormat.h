#pragma once

#include "Hazel/Asset/Asset.h"

#include <cstdint>

namespace Hazel {

	struct ProjectRuntimeInfo
	{
		struct FileHeader
		{
			const char HEADER[4] = { 'H', 'D', 'A', 'T' };
			uint32_t Version = 1;
		};

		FileHeader Header;
		AssetHandle StartScene = 0;
	};

}
