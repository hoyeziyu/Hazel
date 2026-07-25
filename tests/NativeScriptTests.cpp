#include <gtest/gtest.h>

#include <filesystem>

#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Script/NativeScriptFactory.h"
#include "Hazel/Script/NativeScriptRegistry.h"
#include "Hazel/Script/FlyCameraControllerScript.h"

TEST(NativeScriptTest, FactoryBindsRegisteredScript)
{
	Hazel::RegisterBuiltInNativeScripts();

	Hazel::NativeScriptComponent component;
	component.ClassName = "FlyCameraController";
	ASSERT_TRUE(Hazel::NativeScriptFactory::Bind(component, component.ClassName));
	ASSERT_NE(component.InstantiateScript, nullptr);

	Hazel::ScriptableEntity* instance = component.InstantiateScript();
	ASSERT_NE(instance, nullptr);
	delete instance;
}

TEST(NativeScriptTest, SerializeClassNameRoundTrip)
{
	Hazel::RegisterBuiltInNativeScripts();

	auto scene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::Entity entity = scene->CreateEntity("ScriptEntity");
	const Hazel::UUID entityID = entity.GetComponent<Hazel::IDComponent>().ID;
	auto& script = entity.AddComponent<Hazel::NativeScriptComponent>();
	script.ClassName = "JumpController";

	const std::filesystem::path path =
		std::filesystem::temp_directory_path() / "hazel_native_script_test.yaml";

	Hazel::SceneSerializer writer(scene);
	writer.Serialize(path.string());

	auto loadedScene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::SceneSerializer reader(loadedScene);
	ASSERT_TRUE(reader.Deserialize(path.string()));

	Hazel::Entity loaded = loadedScene->GetEntityWithUUID(entityID);
	ASSERT_TRUE(loaded);
	ASSERT_TRUE(loaded.HasComponent<Hazel::NativeScriptComponent>());
	EXPECT_EQ(loaded.GetComponent<Hazel::NativeScriptComponent>().ClassName, "JumpController");

	std::error_code ec;
	std::filesystem::remove(path, ec);
}
