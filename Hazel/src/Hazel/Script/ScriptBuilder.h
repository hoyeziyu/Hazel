#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Project/Project.h"

namespace Hazel {

	class ScriptBuilder
	{
	public:
		static bool BuildScriptAssembly(const Ref<Project>& project);
	};

}
