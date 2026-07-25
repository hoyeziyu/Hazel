#pragma once

#include "Hazel/Scene/Components.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Hazel {

	class Scene;

	class NativeScriptFactory
	{
	public:
		using BindFn = std::function<void(NativeScriptComponent&)>;

		template<typename T>
		static void Register(const std::string& name)
		{
			GetRegistry()[name] = [](NativeScriptComponent& nsc) { nsc.Bind<T>(); };
		}

		static bool Bind(NativeScriptComponent& component, const std::string& name);
		static void BindSceneScripts(Scene& scene);
		static const std::vector<std::string>& GetRegisteredNames();

	private:
		static std::unordered_map<std::string, BindFn>& GetRegistry();
	};

}
