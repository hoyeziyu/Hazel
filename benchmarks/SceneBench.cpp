#include <benchmark/benchmark.h>

#include "Hazel/Asset/MaterialAsset.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Components.h"

static void BM_SceneCreateEntity(benchmark::State& state)
{
	Hazel::Scene scene;
	for (auto _ : state)
	{
		auto entity = scene.CreateEntity("Entity");
		benchmark::DoNotOptimize(entity);
	}
}
BENCHMARK(BM_SceneCreateEntity);

static void BM_TransformGetTransform(benchmark::State& state)
{
	Hazel::TransformComponent transform;
	transform.Translation = glm::vec3(1.0f, 2.0f, 3.0f);
	transform.Rotation = glm::vec3(0.1f, 0.2f, 0.3f);
	transform.Scale = glm::vec3(1.5f, 1.5f, 1.5f);

	for (auto _ : state)
	{
		glm::mat4 matrix = transform.GetTransform();
		benchmark::DoNotOptimize(matrix);	// 利用“优化障碍["optimization barriers"]”来诱使编译器执行我们真正想要测量的操作

		// Force the compiler to forget what 
    	// it knows about memory 
    	// benchmark::ClobberMemory();	// 强制编译器忘记它对内存的了解
	}
}
BENCHMARK(BM_TransformGetTransform);

static void BM_MaterialAssetLookup(benchmark::State& state)
{
	Hazel::Ref<Hazel::MaterialAsset> material = Hazel::MaterialAsset::Create();
	material->AlbedoColor = glm::vec3(0.5f, 0.6f, 0.7f);
	material->AlbedoMap = Hazel::AssetHandle(12345);

	for (auto _ : state)
	{
		glm::vec3 color = material->AlbedoColor;
		bool hasMap = material->AlbedoMap != Hazel::AssetHandle(0);
		benchmark::DoNotOptimize(color);
		benchmark::DoNotOptimize(hasMap);
	}
}
BENCHMARK(BM_MaterialAssetLookup);
