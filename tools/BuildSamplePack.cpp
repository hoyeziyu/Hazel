#include "Hazel/Core/Log.h"
#include "Hazel/Core/Window.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Project/ProjectSerializer.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Serialization/AssetPack.h"

#include <spdlog/spdlog.h>

#include <atomic>
#include <filesystem>
#include <iostream>

int main(int argc, char** argv)
{
	std::cerr << "BuildSamplePack starting..." << std::endl;

	spdlog::drop_all();
	Hazel::Log::Init();

	std::filesystem::path projectDirectory = argc >= 2 ? argv[1] : "Hazelnut/SampleProject";
	if (!projectDirectory.is_absolute())
		projectDirectory = std::filesystem::current_path() / projectDirectory;

	std::cerr << "Project directory: " << projectDirectory.string() << std::endl;

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

	std::cerr << "Project file: " << projectFile.string() << std::endl;

	auto project = Hazel::CreateRef<Hazel::Project>();
	Hazel::ProjectSerializer serializer(project);
	if (!serializer.Deserialize(projectFile))
		return 1;

	std::cerr << "Project deserialized, activating editor..." << std::endl;
	Hazel::Project::SetActiveEditor(project);
	std::cerr << "Active project set, creating GL context..." << std::endl;

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
	std::cout << "Wrote " << (project->GetAssetDirectory() / "Project.hdat").string() << std::endl;

	Hazel::Renderer::Shutdown();
	Hazel::Project::ClearActive();
	return 0;
}
