#include "hzpch.h"
#include "NativeScriptFactory.h"

#include "Hazel/Scene/Scene.h"

namespace Hazel {

	std::unordered_map<std::string, NativeScriptFactory::BindFn>& NativeScriptFactory::GetRegistry()
	{
		static std::unordered_map<std::string, BindFn> registry;
		return registry;
	}

	bool NativeScriptFactory::Bind(NativeScriptComponent& component, const std::string& name)
	{
		auto& registry = GetRegistry();
		auto it = registry.find(name);
		if (it == registry.end())
			return false;

		it->second(component);
		return true;
	}

	void NativeScriptFactory::BindSceneScripts(Scene& scene)
	{
		scene.m_Registry.view<NativeScriptComponent>().each([](auto, NativeScriptComponent& nsc)
		{
			if (nsc.ClassName.empty() || nsc.InstantiateScript)
				return;

			if (!Bind(nsc, nsc.ClassName))
				HZ_CORE_WARN("Unknown native script class: {}", nsc.ClassName);
		});
	}

	const std::vector<std::string>& NativeScriptFactory::GetRegisteredNames()
	{
		static std::vector<std::string> names;
		names.clear();
		names.reserve(GetRegistry().size());
		for (const auto& [name, _] : GetRegistry())
			names.push_back(name);
		return names;
	}

}
