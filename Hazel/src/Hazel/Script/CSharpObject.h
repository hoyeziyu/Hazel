#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Core/Log.h"

#include <Coral/ManagedObject.hpp>

namespace Hazel {

	class CSharpObject
	{
	public:
		template<typename... TArgs>
		void Invoke(std::string_view methodName, TArgs&&... args)
		{
			HZ_CORE_ASSERT(m_Handle != nullptr, "Invalid C# script instance");
			m_Handle->InvokeMethod(methodName, std::forward<TArgs>(args)...);
		}

		bool IsValid() const { return m_Handle != nullptr; }

	private:
		Coral::ManagedObject* m_Handle = nullptr;
		friend class ScriptEngine;
	};

}
