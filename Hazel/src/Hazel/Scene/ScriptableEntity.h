#pragma once

#include "Entity.h"
#include "Hazel/Core/Timestep.h"

namespace Hazel {

	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity() {}

		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

		template<typename T>
		bool HasComponent() const
		{
			return m_Entity.HasComponent<T>();
		}

		Entity GetEntity() const { return m_Entity; }

	protected:
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate([[maybe_unused]] Timestep ts) {}

	private:
		Entity m_Entity;

		friend class Scene;
	};

}
