#pragma once

#include "Hazel/Core/Layer.h"

#include <filesystem>
#include <string>

namespace Hazel {

	class LauncherLayer : public Layer
	{
	public:
		LauncherLayer();
		~LauncherLayer() override = default;

		void OnAttach() override;
		void OnImGuiRender() override;

	private:
		void LaunchRuntime();
		std::filesystem::path GetRuntimeExecutablePath() const;

		char m_ProjectPathBuffer[512] = "SampleProject";
		std::string m_StatusMessage;
	};

}
