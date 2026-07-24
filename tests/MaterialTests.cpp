#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "Hazel/Asset/MaterialAsset.h"
#include "Hazel/Asset/AssetSerializer.h"
#include "Hazel/Asset/AssetMetadata.h"

TEST(MaterialAssetTest, SerializeAndDeserializeRoundTrip)
{
	auto material = Hazel::MaterialAsset::Create();
	material->AlbedoColor = glm::vec3(0.2f, 0.4f, 0.8f);
	material->Metalness = 0.25f;
	material->Roughness = 0.75f;
	material->Emission = 0.1f;
	material->AlbedoMap = Hazel::AssetHandle(4890635141521051159);

	Hazel::MaterialAssetSerializer serializer;
	const std::string yaml = serializer.SerializeToYAML(material);

	Hazel::Ref<Hazel::MaterialAsset> loaded;
	ASSERT_TRUE(serializer.DeserializeFromYAML(yaml, loaded));
	EXPECT_EQ(loaded->AlbedoColor, material->AlbedoColor);
	EXPECT_FLOAT_EQ(loaded->Metalness, material->Metalness);
	EXPECT_FLOAT_EQ(loaded->Roughness, material->Roughness);
	EXPECT_FLOAT_EQ(loaded->Emission, material->Emission);
	EXPECT_EQ(loaded->AlbedoMap, material->AlbedoMap);
}

TEST(MaterialAssetTest, DeserializeHazelEngineCompatibleYaml)
{
	const std::string yaml = R"(
Material:
  AlbedoColor: [1, 1, 1]
  Metalness: 0
  Roughness: 0.5
  Emission: 0
  AlbedoMap: 4890635141521051159
)";

	Hazel::MaterialAssetSerializer serializer;
	Hazel::Ref<Hazel::MaterialAsset> material;
	ASSERT_TRUE(serializer.DeserializeFromYAML(yaml, material));
	EXPECT_EQ(material->AlbedoMap, Hazel::AssetHandle(4890635141521051159));
	EXPECT_FLOAT_EQ(material->Roughness, 0.5f);
}
