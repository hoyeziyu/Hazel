#include "ScriptFieldDrawer.h"

#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Hazel {

	bool DrawScriptFieldValue(const Ref<Scene>& scene, FieldStorage& storage)
	{
		ImGui::PushID(storage.GetName().data());
		bool modified = false;

		switch (storage.GetType())
		{
		case DataType::Float:
		{
			float value = storage.GetValue<float>();
			if (ImGui::DragFloat(storage.GetName().data(), &value, 0.01f))
			{
				storage.SetValue(value);
				modified = true;
			}
			break;
		}
		case DataType::Int:
		{
			int32_t value = storage.GetValue<int32_t>();
			if (ImGui::DragInt(storage.GetName().data(), &value))
			{
				storage.SetValue(value);
				modified = true;
			}
			break;
		}
		case DataType::Bool:
		{
			bool value = (bool)storage.GetValue<Coral::Bool32>();
			if (ImGui::Checkbox(storage.GetName().data(), &value))
			{
				storage.SetValue(Coral::Bool32(value));
				modified = true;
			}
			break;
		}
		case DataType::Vector2:
		{
			glm::vec2 value = storage.GetValue<glm::vec2>();
			if (ImGui::DragFloat2(storage.GetName().data(), glm::value_ptr(value), 0.01f))
			{
				storage.SetValue(value);
				modified = true;
			}
			break;
		}
		case DataType::Vector3:
		{
			glm::vec3 value = storage.GetValue<glm::vec3>();
			if (ImGui::DragFloat3(storage.GetName().data(), glm::value_ptr(value), 0.01f))
			{
				storage.SetValue(value);
				modified = true;
			}
			break;
		}
		case DataType::Entity:
		{
			UUID entityID = storage.GetValue<UUID>();
			std::string preview = "None";
			if (entityID && scene)
			{
				Entity entity = scene->GetEntityWithUUID(entityID);
				if (entity && entity.HasComponent<TagComponent>())
					preview = entity.GetComponent<TagComponent>().Tag;
			}

			if (ImGui::BeginCombo(storage.GetName().data(), preview.c_str()))
			{
				if (ImGui::Selectable("None", entityID == UUID(0)))
				{
					storage.SetValue(UUID(0));
					modified = true;
				}

				if (scene)
				{
					auto view = scene->GetAllEntityUUIDs();
					for (UUID id : view)
					{
						Entity entity = scene->GetEntityWithUUID(id);
						if (!entity || !entity.HasComponent<TagComponent>())
							continue;

						const std::string& tag = entity.GetComponent<TagComponent>().Tag;
						if (ImGui::Selectable(tag.c_str(), entityID == id))
						{
							storage.SetValue(id);
							modified = true;
						}
					}
				}

				ImGui::EndCombo();
			}
			break;
		}
		default:
			ImGui::TextDisabled("%.*s (unsupported)", (int)storage.GetName().size(), storage.GetName().data());
			break;
		}

		ImGui::PopID();
		return modified;
	}

}
