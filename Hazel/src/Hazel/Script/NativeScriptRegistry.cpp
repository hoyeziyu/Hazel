#include "hzpch.h"
#include "NativeScriptRegistry.h"

#include "FlyCameraControllerScript.h"
#include "JumpControllerScript.h"
#include "NativeScriptFactory.h"

namespace Hazel {

	void RegisterBuiltInNativeScripts()
	{
		static bool registered = false;
		if (registered)
			return;
		registered = true;

		NativeScriptFactory::Register<FlyCameraControllerScript>("FlyCameraController");
		NativeScriptFactory::Register<JumpControllerScript>("JumpController");
	}

}
