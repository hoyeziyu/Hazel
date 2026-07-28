#pragma once

#include "Hazel/Core/Buffer.h"
#include "Hazel/Core/Core.h"
#include "SoundBank.h"

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
		uint32_t PlaySound(AssetHandle audioHandle, float volume = 1.0f, bool loop = false);
		uint32_t PlaySoundConfig(AssetHandle soundConfigHandle);
		void StopSound(uint32_t handle);
		void StopAll();

		bool LoadSoundBank(const std::filesystem::path& path);
		bool BuildSoundBank(const std::filesystem::path& path);
		const Ref<SoundBank>& GetSoundBank() const { return m_SoundBank; }

		bool IsInitialized() const { return m_Initialized; }

	private:
		AudioEngine() = default;

		struct SoundInstance;
		uint32_t PlaySoundFromMemory(Buffer&& audioData, float volume, bool loop);

	private:
		bool m_Initialized = false;
		void* m_Engine = nullptr;
		Ref<SoundBank> m_SoundBank;
		std::unordered_map<uint32_t, SoundInstance*> m_Sounds;
		uint32_t m_NextHandle = 1;
	};

}
