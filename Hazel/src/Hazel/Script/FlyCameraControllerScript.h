#pragma once

#include "Hazel/Scene/ScriptableEntity.h"

namespace Hazel {

	class FlyCameraControllerScript : public ScriptableEntity
	{
	protected:
		void OnUpdate(Timestep ts) override;
	};

}
