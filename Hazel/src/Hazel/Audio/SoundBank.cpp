#include "hzpch.h"
#include "SoundBank.h"

#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Asset/AssetManager/EditorAssetManager.h"
#include "Hazel/Asset/AudioFile.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Serialization/FileStream.h"

namespace Hazel {

	namespace {

		Buffer ReadFileBytes(const std::filesystem::path& path)
		{
			std::ifstream stream(path, std::ios::binary);
			stream.seekg(0, std::ios::end);
			const auto size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			Buffer buffer;
			buffer.Allocate((uint64_t)size);
			if (size > 0)
				stream.read((char*)buffer.Data, size);
			return buffer;
		}

	}

	SoundBank::SoundBank(const std::filesystem::path& path)
		: m_Path(path)
	{
		FileStreamReader reader(path);
		if (!reader.IsStreamGood())
			return;

		reader.ReadRaw(m_File.Header);
		if (std::memcmp(m_File.Header.HEADER, "HZSB", 4) != 0)
			return;

		if (m_File.Header.Version != SoundBankFile::CurrentVersion)
			return;

		for (uint32_t i = 0; i < m_File.Header.AudioFileCount; ++i)
		{
			AssetHandle handle = 0;
			reader.ReadRaw(handle);

			SoundBankFile::AudioFileInfo info{};
			reader.ReadRaw(info);
			m_File.Index[handle] = info;
		}

		m_Loaded = true;
		HZ_CORE_INFO("[Audio] Loaded SoundBank '{}' ({} files)", path.string(), m_File.Header.AudioFileCount);
	}

	bool SoundBank::Contains(AssetHandle audioHandle) const
	{
		return m_Loaded && m_File.Index.find(audioHandle) != m_File.Index.end();
	}

	bool SoundBank::ReadAudioData(AssetHandle audioHandle, Buffer& outBuffer) const
	{
		if (!Contains(audioHandle))
			return false;

		const auto& info = m_File.Index.at(audioHandle);
		FileStreamReader reader(m_Path);
		if (!reader.IsStreamGood())
			return false;

		reader.SetStreamPosition(info.DataOffset);
		outBuffer.Allocate(info.PackedSize);
		if (!reader.ReadData((char*)outBuffer.Data, (size_t)info.PackedSize))
		{
			outBuffer.Release();
			return false;
		}

		return true;
	}

	Ref<SoundBank> SoundBank::Create(const std::vector<AssetHandle>& waveAssets, const std::filesystem::path& path)
	{
		if (waveAssets.empty())
		{
			HZ_CORE_WARN("[Audio] SoundBank build aborted: no wave assets referenced");
			return nullptr;
		}

		if (path.empty())
		{
			HZ_CORE_WARN("[Audio] SoundBank build aborted: output path is empty");
			return nullptr;
		}

		if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
		{
			HZ_CORE_WARN("[Audio] SoundBank build aborted: output path is a directory");
			return nullptr;
		}

		if (std::filesystem::exists(path))
			std::filesystem::remove(path);

		auto soundBank = CreateRef<SoundBank>();
		soundBank->m_Path = path;
		auto& soundBankFile = soundBank->m_File;

		soundBankFile.Header.Version = SoundBankFile::CurrentVersion;

		FileStreamWriter serializer(path);
		if (!serializer.IsStreamGood())
			return nullptr;

		serializer.WriteRaw(soundBankFile.Header);

		const uint64_t indexEntrySize = sizeof(AssetHandle) + sizeof(SoundBankFile::AudioFileInfo);
		const uint64_t indexTableSize = indexEntrySize * waveAssets.size();
		const uint64_t indexTablePos = serializer.GetStreamPosition();
		serializer.WriteZero(indexTableSize);

		for (const AssetHandle handle : waveAssets)
		{
			const auto audioFile = AssetManager::GetAsset<AudioFile>(handle);
			if (!audioFile)
			{
				HZ_CORE_WARN("[Audio] Skipping invalid audio asset handle {}", (uint64_t)handle);
				continue;
			}

			const auto& metadata = Project::GetActiveAssetManager()->GetMetadata(handle);
			const auto fileSystemPath = Project::GetActiveAssetManager()->GetFileSystemPath(metadata);
			if (!std::filesystem::exists(fileSystemPath))
			{
				HZ_CORE_WARN("[Audio] Skipping missing audio file '{}'", fileSystemPath.string());
				continue;
			}

			SoundBankFile::AudioFileInfo info{};
			info.DataOffset = serializer.GetStreamPosition();
			info.PackedSize = audioFile->FileSize;

			if (auto optInfo = AudioFileUtils::GetFileInfo(fileSystemPath))
				info.Info = *optInfo;

			soundBankFile.Index[handle] = info;

			Buffer encodedFile = ReadFileBytes(fileSystemPath);
			serializer.WriteBuffer(encodedFile, false);
			encodedFile.Release();
		}

		serializer.SetStreamPosition(indexTablePos);
		for (const auto& [assetHandle, audioFileInfo] : soundBankFile.Index)
		{
			serializer.WriteRaw(assetHandle);
			serializer.WriteRaw(audioFileInfo);
		}

		soundBankFile.Header.AudioFileCount = (uint32_t)soundBankFile.Index.size();
		serializer.SetStreamPosition(0);
		serializer.WriteRaw(soundBankFile.Header);

		soundBank->m_Loaded = !soundBankFile.Index.empty();
		HZ_CORE_INFO("[Audio] Built SoundBank '{}' ({} files)", path.string(), soundBankFile.Index.size());
		return soundBank;
	}

	Ref<SoundBank> SoundBank::Load(const std::filesystem::path& path)
	{
		auto bank = CreateRef<SoundBank>(path);
		if (!bank->IsLoaded())
			return nullptr;
		return bank;
	}

}
