#pragma once

#include "Hazel/Scene/ScriptableEntity.h"

namespace Hazel {

	class JumpControllerScript : public ScriptableEntity
	{
	protected:
		void OnUpdate(Timestep ts) override;
	};

}
