#include <gtest/gtest.h>

#include <glm/gtc/epsilon.hpp>

#include "Hazel/Scene/Components.h"

TEST(TransformComponentTest, GetTransformAppliesTranslation)
{
	Hazel::TransformComponent transform;
	transform.Translation = glm::vec3(2.0f, 3.0f, 4.0f);

	const glm::mat4 matrix = transform.GetTransform();
	const glm::vec4 translated = matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	EXPECT_NEAR(translated.x, 2.0f, 1e-4f);
	EXPECT_NEAR(translated.y, 3.0f, 1e-4f);
	EXPECT_NEAR(translated.z, 4.0f, 1e-4f);
}

TEST(TransformComponentTest, GetTransformAppliesScale)
{
	Hazel::TransformComponent transform;
	transform.Scale = glm::vec3(2.0f, 2.0f, 2.0f);

	const glm::mat4 matrix = transform.GetTransform();
	const glm::vec4 scaled = matrix * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	EXPECT_NEAR(scaled.x, 2.0f, 1e-4f);
	EXPECT_NEAR(scaled.y, 2.0f, 1e-4f);
	EXPECT_NEAR(scaled.z, 2.0f, 1e-4f);
}
