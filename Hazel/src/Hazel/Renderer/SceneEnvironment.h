#pragma once

#include <glm/glm.hpp>

namespace Hazel {

	struct SceneEnvironmentData
	{
		glm::vec3 CameraPosition{ 0.0f, 0.0f, 0.0f };
		glm::vec3 LightDirection{ 0.0f, -1.0f, 0.0f };
		glm::vec3 LightRadiance{ 1.0f, 1.0f, 1.0f };
		float LightIntensity = 1.0f;
		bool HasDirectionalLight = false;
	};

	struct SceneRendererStats
	{
		uint32_t DrawCalls = 0;
		uint32_t TriangleCount = 0;

		void Reset()
		{
			DrawCalls = 0;
			TriangleCount = 0;
		}
	};

}
