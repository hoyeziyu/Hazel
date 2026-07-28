#include "hzpch.h"
#include "AudioEngine.h"

#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Asset/AudioFile.h"
#include "Hazel/Asset/SoundConfigAsset.h"
#include "Hazel/Project/Project.h"

#include <unordered_set>
#include <vector>

#ifdef HZ_AUDIO

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace Hazel {

	struct AudioEngine::SoundInstance
	{
		Buffer AudioData{};
		ma_decoder Decoder{};
		ma_sound Sound{};
		bool Loop = false;
		bool UsesMemoryDecoder = false;
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
		m_SoundBank = nullptr;

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
				if (instance->UsesMemoryDecoder)
					ma_decoder_uninit(&instance->Decoder);
				instance->AudioData.Release();
				delete instance;
				it = m_Sounds.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	uint32_t AudioEngine::PlaySoundFromMemory(Buffer&& audioData, float volume, bool loop)
	{
		if (!m_Initialized || !audioData)
			return 0;

		auto* instance = new SoundInstance();
		instance->Loop = loop;
		instance->AudioData = std::move(audioData);
		instance->UsesMemoryDecoder = true;

		const ma_result decoderResult = ma_decoder_init_memory(
			instance->AudioData.Data,
			(size_t)instance->AudioData.Size,
			nullptr,
			&instance->Decoder);

		if (decoderResult != MA_SUCCESS)
		{
			HZ_CORE_WARN("[Audio] Failed to decode sound from memory (code={})", (int)decoderResult);
			instance->AudioData.Release();
			delete instance;
			return 0;
		}

		const ma_result soundResult = ma_sound_init_from_data_source(
			(ma_engine*)m_Engine,
			&instance->Decoder,
			MA_SOUND_FLAG_DECODE,
			nullptr,
			&instance->Sound);

		if (soundResult != MA_SUCCESS)
		{
			HZ_CORE_WARN("[Audio] Failed to init sound from memory (code={})", (int)soundResult);
			ma_decoder_uninit(&instance->Decoder);
			instance->AudioData.Release();
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
			0,
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

	uint32_t AudioEngine::PlaySound(AssetHandle audioHandle, float volume, bool loop)
	{
		if (!m_Initialized || (uint64_t)audioHandle == 0)
			return 0;

		if (m_SoundBank && m_SoundBank->Contains(audioHandle))
		{
			Buffer buffer;
			if (m_SoundBank->ReadAudioData(audioHandle, buffer))
				return PlaySoundFromMemory(std::move(buffer), volume, loop);
		}

		const auto& metadata = AssetManager::GetMetadata(audioHandle);
		if (!metadata.IsValid())
		{
			HZ_CORE_WARN("[Audio] Invalid audio asset handle {}", (uint64_t)audioHandle);
			return 0;
		}

		auto project = Project::GetActive();
		if (!project)
			return 0;

		return PlaySound(project->GetAssetPath(metadata.FilePath), volume, loop);
	}

	uint32_t AudioEngine::PlaySoundConfig(AssetHandle soundConfigHandle)
	{
		if ((uint64_t)soundConfigHandle == 0)
			return 0;

		const auto config = AssetManager::GetAsset<SoundConfigAsset>(soundConfigHandle);
		if (!config || (uint64_t)config->DataSourceAsset == 0)
		{
			HZ_CORE_WARN("[Audio] Invalid SoundConfig handle {}", (uint64_t)soundConfigHandle);
			return 0;
		}

		return PlaySound(config->DataSourceAsset, config->VolumeMultiplier, config->IsLooping);
	}

	void AudioEngine::StopSound(uint32_t handle)
	{
		auto it = m_Sounds.find(handle);
		if (it == m_Sounds.end())
			return;

		ma_sound_stop(&it->second->Sound);
		ma_sound_uninit(&it->second->Sound);
		if (it->second->UsesMemoryDecoder)
			ma_decoder_uninit(&it->second->Decoder);
		it->second->AudioData.Release();
		delete it->second;
		m_Sounds.erase(it);
	}

	void AudioEngine::StopAll()
	{
		for (auto& [handle, instance] : m_Sounds)
		{
			ma_sound_stop(&instance->Sound);
			ma_sound_uninit(&instance->Sound);
			if (instance->UsesMemoryDecoder)
				ma_decoder_uninit(&instance->Decoder);
			instance->AudioData.Release();
			delete instance;
		}
		m_Sounds.clear();
	}

	bool AudioEngine::LoadSoundBank(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			HZ_CORE_WARN("[Audio] SoundBank not found: {}", path.string());
			return false;
		}

		m_SoundBank = SoundBank::Load(path);
		return m_SoundBank != nullptr;
	}

	bool AudioEngine::BuildSoundBank(const std::filesystem::path& path)
	{
		std::unordered_set<AssetHandle> waveSet;

		for (AssetHandle handle : AssetManager::GetAllAssetsWithType(AssetType::SoundConfig))
		{
			const auto config = AssetManager::GetAsset<SoundConfigAsset>(handle);
			if (!config || (uint64_t)config->DataSourceAsset == 0)
				continue;

			if (AssetManager::GetAssetType(config->DataSourceAsset) == AssetType::Audio)
				waveSet.insert(config->DataSourceAsset);
		}

		if (waveSet.empty())
		{
			HZ_CORE_WARN("[Audio] No wave assets referenced by SoundConfig assets; SoundBank build aborted");
			return false;
		}

		std::vector<AssetHandle> waveAssets(waveSet.begin(), waveSet.end());
		m_SoundBank = SoundBank::Create(waveAssets, path);
		return m_SoundBank != nullptr;
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

	uint32_t AudioEngine::PlaySound(AssetHandle, float, bool)
	{
		return 0;
	}

	uint32_t AudioEngine::PlaySoundConfig(AssetHandle)
	{
		return 0;
	}

	uint32_t AudioEngine::PlaySoundFromMemory(Buffer&&, float, bool)
	{
		return 0;
	}

	void AudioEngine::StopSound(uint32_t) {}
	void AudioEngine::StopAll() {}

	bool AudioEngine::LoadSoundBank(const std::filesystem::path&)
	{
		return false;
	}

	bool AudioEngine::BuildSoundBank(const std::filesystem::path&)
	{
		return false;
	}

}

#endif
