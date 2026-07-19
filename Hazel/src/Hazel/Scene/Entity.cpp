#include "hzpch.h"
#include "Entity.h"
#include "Components.h"

namespace Hazel {

	Entity::Entity(entt::entity handle, Scene* scene)
		:m_EntityHandle(handle), m_Scene(scene)
	{

	}

	UUID Entity::GetUUID() const
	{
		HZ_CORE_ASSERT(HasComponent<IDComponent>(), "Entity does not have IDComponent!");
		return GetComponent<IDComponent>().ID;
	}

}