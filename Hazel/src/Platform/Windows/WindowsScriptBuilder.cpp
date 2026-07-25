#include "hzpch.h"
#include "Hazel/Script/ScriptBuilder.h"

#include "Hazel/Core/Log.h"

#include <cstdlib>
#include <format>

namespace Hazel {

	bool ScriptBuilder::BuildScriptAssembly(const Ref<Project>& project)
	{
		if (!project)
		{
			HZ_CORE_ERROR("Cannot build scripts without an active project.");
			return false;
		}

		const auto csproj = project->GetScriptProjectPath();
		if (!std::filesystem::exists(csproj))
		{
			HZ_CORE_ERROR("Script project not found: {}", csproj.string());
			return false;
		}

		const auto outputPath = project->GetScriptModulePath();
		std::error_code ec;
		std::filesystem::create_directories(outputPath, ec);

		const auto intermediatePath = outputPath / "Intermediates";
		std::filesystem::create_directories(intermediatePath, ec);

		const std::string command = std::format(
			"dotnet build \"{}\" -c Debug --nologo"
			" -p:OutputPath=\"{}\""
			" -p:IntermediateOutputPath=\"{}\"",
			csproj.string(),
			(outputPath / "").string(),
			(intermediatePath / "").string());

		HZ_CORE_INFO("[Scripting] Building game scripts: {}", csproj.filename().string());
		const int result = std::system(command.c_str());
		if (result != 0)
		{
			HZ_CORE_ERROR("[Scripting] dotnet build failed (exit code {})", result);
			return false;
		}

		HZ_CORE_INFO("[Scripting] Built scripts to {}", outputPath.string());
		return true;
	}

}
