#pragma once

#include <map>

#include "Hazel/Asset/Asset.h"

namespace Hazel {

	struct AssetPackFile
	{
		struct AssetInfo
		{
			uint64_t PackedOffset = 0;
			uint64_t PackedSize = 0;
			uint16_t Type = 0;
			uint16_t Flags = 0;
		};

		struct SceneInfo
		{
			uint64_t PackedOffset = 0;
			uint64_t PackedSize = 0;
			uint16_t Flags = 0;
			std::map<uint64_t, AssetInfo> Assets;
		};

		struct IndexTable
		{
			uint64_t PackedAppBinaryOffset = 0;
			uint64_t PackedAppBinarySize = 0;
			std::map<uint64_t, SceneInfo> Scenes;
		};

		struct FileHeader
		{
			const char HEADER[4] = { 'H','Z','A','P' };
			uint32_t Version = 1;
			uint64_t BuildVersion = 0;
		};

		FileHeader Header;
		IndexTable Index;
	};

}
