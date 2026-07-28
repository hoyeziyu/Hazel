#pragma once

#include "Hazel/Core/Layer.h"
#include "Hazel/Core/Core.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Renderer/SceneRenderer.h"

namespace Hazel {

	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer(int argc, char** argv);
		~RuntimeLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& event) override;

	private:
		bool LoadProject(const std::filesystem::path& projectDirectory);
		std::filesystem::path ResolveProjectDirectory(int argc, char** argv) const;
		std::filesystem::path FindProjectFile(const std::filesystem::path& projectDirectory) const;

		int m_Argc = 0;
		char** m_Argv = nullptr;

		Ref<Scene> m_Scene;
		Ref<SceneRenderer> m_SceneRenderer;
		bool m_ProjectLoaded = false;
	};

}
