#include <benchmark/benchmark.h>

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
		benchmark::DoNotOptimize(matrix);
	}
}
BENCHMARK(BM_TransformGetTransform);
