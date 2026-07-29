#include "Hazel/Core/Log.h"
#include "Hazel/Core/Window.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Project/ProjectSerializer.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Serialization/AssetPack.h"

#include <atomic>
#include <filesystem>
#include <iostream>

#ifdef HZ_PLATFORM_WINDOWS
#include <Windows.h>
#endif

int main(int argc, char** argv)
{
	std::filesystem::path projectDirectory = argc >= 2 ? argv[1] : "Hazelnut/SampleProject";
	if (!projectDirectory.is_absolute())
		projectDirectory = std::filesystem::current_path() / projectDirectory;

#ifdef HZ_PLATFORM_WINDOWS
	char exePath[MAX_PATH];
	GetModuleFileNameA(nullptr, exePath, MAX_PATH);
	std::filesystem::current_path(std::filesystem::path(exePath).parent_path());
#endif

	Hazel::Log::InitHeadless();

	std::filesystem::path projectFile;
	for (const auto& entry : std::filesystem::directory_iterator(projectDirectory))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".hzproj")
		{
			projectFile = entry.path();
			break;
		}
	}

	if (projectFile.empty())
	{
		std::cerr << "No .hzproj found in " << projectDirectory << std::endl;
		return 1;
	}

	auto project = Hazel::CreateRef<Hazel::Project>();
	Hazel::ProjectSerializer serializer(project);
	if (!serializer.Deserialize(projectFile))
		return 1;

	Hazel::Project::SetActiveEditor(project);

	Hazel::WindowProps props("BuildSamplePack", 64, 64);
	auto window = Hazel::Window::Create(props);
	(void)window;
	Hazel::Renderer::Init();

	const auto outputPath = project->GetAssetDirectory() / "AssetPack.hap";
	std::cout << "Building pack to " << outputPath.string() << std::endl;
	std::atomic<float> progress = 0.0f;
	if (!Hazel::AssetPack::CreateFromActiveProject(outputPath, progress))
	{
		std::cerr << "AssetPack build failed" << std::endl;
		return 1;
	}

	std::cout << "Built " << outputPath.string() << std::endl;

	const auto runtimePath = project->GetAssetDirectory() / "Project.hdat";
	if (std::filesystem::exists(runtimePath))
		std::cout << "Wrote " << runtimePath.string() << std::endl;

	const auto soundBankPath = project->GetAssetDirectory() / "SoundBank.hsb";
	if (std::filesystem::exists(soundBankPath))
		std::cout << "Wrote " << soundBankPath.string() << std::endl;
	else
		std::cerr << "SoundBank not written (no SoundConfig wav references)" << std::endl;

	Hazel::Renderer::Shutdown();
	Hazel::Project::ClearActive();
	return 0;
}
