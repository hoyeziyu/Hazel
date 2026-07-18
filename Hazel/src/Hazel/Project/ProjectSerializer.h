#pragma once

#include "Project.h"

namespace Hazel {

	class ProjectSerializer
	{
	public:
		explicit ProjectSerializer(const Ref<Project>& project);

		void Serialize(const std::filesystem::path& filepath);
		bool Deserialize(const std::filesystem::path& filepath);

	private:
		Ref<Project> m_Project;
	};

}
