#include "hzpch.h"
#include "JumpControllerScript.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Physics2D/Physics2DScene.h"
#include "Hazel/Scene/Components.h"

namespace Hazel {

	void JumpControllerScript::OnUpdate(Timestep ts)
	{
		(void)ts;
		if (!HasComponent<RigidBody2DComponent>())
			return;

		if (Input::IsKeyPressed(HZ_KEY_SPACE))
			Physics2DScene::ApplyLinearImpulse(GetComponent<RigidBody2DComponent>(), { 0.0f, 6.0f });
	}

}
