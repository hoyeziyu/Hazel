#pragma once

#include "Hazel/Core/Core.h"

#include <cstdint>
#include <filesystem>
#include <unordered_map>

namespace Hazel {

	class AudioEngine
	{
	public:
		static AudioEngine& Get();

		void Init();
		void Shutdown();
		void Update();

		uint32_t PlaySound(const std::filesystem::path& filepath, float volume = 1.0f, bool loop = false);
		void StopSound(uint32_t handle);
		void StopAll();

		bool IsInitialized() const { return m_Initialized; }

	private:
		AudioEngine() = default;

		struct SoundInstance;

	private:
		bool m_Initialized = false;
		void* m_Engine = nullptr;
		std::unordered_map<uint32_t, SoundInstance*> m_Sounds;
		uint32_t m_NextHandle = 1;
	};

}
