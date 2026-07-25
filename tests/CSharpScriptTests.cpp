#include <gtest/gtest.h>

#include <string>

#include "Hazel/Core/Hash.h"
#include "Hazel/Core/Buffer.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Script/ScriptEntityStorage.hpp"

#include <yaml-cpp/yaml.h>

TEST(CSharpScriptTest, ScriptIDHashIsStable)
{
	const uint32_t rotatorHash = Hazel::Hash::GenerateFNVHash("Sample.Rotator");
	const uint32_t moverHash = Hazel::Hash::GenerateFNVHash("Sample.Mover");
	EXPECT_NE(rotatorHash, 0u);
	EXPECT_NE(moverHash, 0u);
	EXPECT_NE(rotatorHash, moverHash);
}

TEST(CSharpScriptTest, ScriptComponentSerializesToYaml)
{
	Hazel::Scene scene;
	Hazel::Entity entity = scene.CreateEntity("ScriptEntity");
	entity.AddComponent<Hazel::ScriptComponent>().ScriptID = Hazel::UUID(12345ull);

	YAML::Emitter out;
	Hazel::SceneSerializer::SerializeEntity(out, entity);
	const std::string yaml = out.c_str();

	EXPECT_NE(yaml.find("ScriptComponent"), std::string::npos);
	EXPECT_NE(yaml.find("ScriptID"), std::string::npos);
	EXPECT_NE(yaml.find("12345"), std::string::npos);
}

TEST(CSharpScriptTest, ScriptFieldStorageSerializesToYaml)
{
	Hazel::Scene scene;
	Hazel::Entity entity = scene.CreateEntity("FieldEntity");
	const Hazel::UUID scriptID = Hazel::UUID(999ull);
	entity.AddComponent<Hazel::ScriptComponent>().ScriptID = scriptID;

	const uint32_t fieldID = Hazel::Hash::GenerateFNVHash("Sample.Rotator.Speed");
	auto& entityStorage = scene.GetScriptStorage().EntityStorage[entity.GetUUID()];
	entityStorage.ScriptID = scriptID;

	float speed = 2.5f;
	Hazel::Buffer speedBuffer(&speed, sizeof(float));
	scene.GetScriptStorage().SetFieldStorage(entity.GetUUID(), fieldID, "Speed", Hazel::DataType::Float, speedBuffer);

	YAML::Emitter out;
	Hazel::SceneSerializer::SerializeEntity(out, entity);
	const std::string yaml = out.c_str();

	EXPECT_NE(yaml.find("Fields"), std::string::npos);
	EXPECT_NE(yaml.find("Speed"), std::string::npos);
	EXPECT_NE(yaml.find("2.5"), std::string::npos);
}
