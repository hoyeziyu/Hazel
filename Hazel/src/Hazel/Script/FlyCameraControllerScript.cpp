#include "hzpch.h"
#include "FlyCameraControllerScript.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Scene/Components.h"

#include <glm/glm.hpp>

namespace Hazel {

	void FlyCameraControllerScript::OnUpdate(Timestep ts)
	{
		auto& transform = GetComponent<TransformComponent>();
		const float speed = 5.0f;
		glm::vec3 velocity{ 0.0f };

		if (Input::IsKeyPressed(HZ_KEY_A))
			velocity.x -= 1.0f;
		if (Input::IsKeyPressed(HZ_KEY_D))
			velocity.x += 1.0f;
		if (Input::IsKeyPressed(HZ_KEY_W))
			velocity.y += 1.0f;
		if (Input::IsKeyPressed(HZ_KEY_S))
			velocity.y -= 1.0f;

		if (glm::dot(velocity, velocity) > 0.0f)
			transform.Translation += glm::normalize(velocity) * speed * ts.GetSeconds();
	}

}
