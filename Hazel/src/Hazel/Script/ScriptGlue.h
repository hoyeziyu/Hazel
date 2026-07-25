#pragma once

namespace Coral {
	class ManagedAssembly;
}

namespace Hazel {

	class ScriptGlue
	{
	public:
		static void RegisterGlue(Coral::ManagedAssembly& coreAssembly);
	};

}
