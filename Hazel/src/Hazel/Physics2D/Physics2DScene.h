#pragma once

#include "Hazel/Core/Timestep.h"
#include "Hazel/Scene/Components.h"

#include <glm/glm.hpp>

namespace Hazel {

	class Scene;

	class Physics2DScene
	{
	public:
		static void Init(Scene& scene);
		static void Shutdown(Scene& scene);
		static void Step(Scene& scene, Timestep ts);
		static bool IsActive(const Scene& scene);
		static void ApplyLinearImpulse(RigidBody2DComponent& rigidBody, const glm::vec2& impulse);
	};

}
