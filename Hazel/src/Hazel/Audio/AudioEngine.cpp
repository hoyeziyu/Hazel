#include "hzpch.h"
#include "AudioEngine.h"

#ifdef HZ_AUDIO

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace Hazel {

	struct AudioEngine::SoundInstance
	{
		ma_sound Sound{};
		bool Loop = false;
	};

	AudioEngine& AudioEngine::Get()
	{
		static AudioEngine instance;
		return instance;
	}

	void AudioEngine::Init()
	{
		if (m_Initialized)
			return;

		auto* engine = new ma_engine();
		const ma_result result = ma_engine_init(nullptr, engine);
		if (result != MA_SUCCESS)
		{
			HZ_CORE_ERROR("[Audio] Failed to initialize miniaudio engine (code={})", (int)result);
			delete engine;
			return;
		}

		m_Engine = engine;
		m_Initialized = true;
		HZ_CORE_INFO("[Audio] miniaudio engine initialized");
	}

	void AudioEngine::Shutdown()
	{
		if (!m_Initialized)
			return;

		StopAll();

		ma_engine_uninit((ma_engine*)m_Engine);
		delete (ma_engine*)m_Engine;
		m_Engine = nullptr;
		m_Initialized = false;
	}

	void AudioEngine::Update()
	{
		if (!m_Initialized)
			return;

		for (auto it = m_Sounds.begin(); it != m_Sounds.end(); )
		{
			auto* instance = it->second;
			if (!instance->Loop && !ma_sound_is_playing(&instance->Sound))
			{
				ma_sound_uninit(&instance->Sound);
				delete instance;
				it = m_Sounds.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	uint32_t AudioEngine::PlaySound(const std::filesystem::path& filepath, float volume, bool loop)
	{
		if (!m_Initialized)
		{
			HZ_CORE_WARN("[Audio] PlaySound called before engine init");
			return 0;
		}

		if (!std::filesystem::exists(filepath))
		{
			HZ_CORE_WARN("[Audio] Sound file not found: {}", filepath.string());
			return 0;
		}

		auto* instance = new SoundInstance();
		instance->Loop = loop;

		const ma_result result = ma_sound_init_from_file(
			(ma_engine*)m_Engine,
			filepath.string().c_str(),
			MA_SOUND_FLAG_NONE,
			nullptr,
			nullptr,
			&instance->Sound);

		if (result != MA_SUCCESS)
		{
			HZ_CORE_WARN("[Audio] Failed to load sound '{}'", filepath.string());
			delete instance;
			return 0;
		}

		ma_sound_set_volume(&instance->Sound, volume);
		ma_sound_set_looping(&instance->Sound, loop ? MA_TRUE : MA_FALSE);
		ma_sound_start(&instance->Sound);

		const uint32_t handle = m_NextHandle++;
		m_Sounds[handle] = instance;
		return handle;
	}

	void AudioEngine::StopSound(uint32_t handle)
	{
		auto it = m_Sounds.find(handle);
		if (it == m_Sounds.end())
			return;

		ma_sound_stop(&it->second->Sound);
		ma_sound_uninit(&it->second->Sound);
		delete it->second;
		m_Sounds.erase(it);
	}

	void AudioEngine::StopAll()
	{
		for (auto& [handle, instance] : m_Sounds)
		{
			ma_sound_stop(&instance->Sound);
			ma_sound_uninit(&instance->Sound);
			delete instance;
		}
		m_Sounds.clear();
	}

}

#else

namespace Hazel {

	AudioEngine& AudioEngine::Get()
	{
		static AudioEngine instance;
		return instance;
	}

	void AudioEngine::Init() {}
	void AudioEngine::Shutdown() {}
	void AudioEngine::Update() {}

	uint32_t AudioEngine::PlaySound(const std::filesystem::path&, float, bool)
	{
		return 0;
	}

	void AudioEngine::StopSound(uint32_t) {}
	void AudioEngine::StopAll() {}

}

#endif
