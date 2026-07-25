#pragma once

#include "Hazel/Core/Timestep.h"

namespace Hazel {

	class Scene;

	class Physics2DScene
	{
	public:
		static void Init(Scene& scene);
		static void Shutdown(Scene& scene);
		static void Step(Scene& scene, Timestep ts);
		static bool IsActive(const Scene& scene);
	};

}
