#pragma once

#include "Hazel/Core/Timestep.h"
#include "Hazel/Events/Event.h"

#include <glm/glm.hpp>

namespace Hazel {

	class EditorCamera
	{
	public:
		EditorCamera(float verticalFovDeg, float width, float height, float nearClip, float farClip);

		void SetActive(bool active) { m_IsActive = active; }
		bool IsActive() const { return m_IsActive; }

		void SetViewportSize(uint32_t width, uint32_t height);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetProjectionMatrix() const { return m_Projection; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }
		glm::vec3 GetPosition() const { return CalculatePosition(); }

	private:
		void UpdateProjection();
		void UpdateView();

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;
		glm::quat GetOrientation() const;
		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

		bool OnMouseScroll(class MouseScrolledEvent& e);

	private:
		glm::mat4 m_Projection = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		float m_VerticalFovDeg = 45.0f;
		float m_AspectRatio = 1.777f;
		float m_NearClip = 0.1f;
		float m_FarClip = 1000.0f;

		uint32_t m_ViewportWidth = 1280;
		uint32_t m_ViewportHeight = 720;

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f;
		float m_Yaw = 0.0f;
		float m_PitchDelta = 0.0f;
		float m_YawDelta = 0.0f;
		glm::vec3 m_PositionDelta = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

		bool m_IsActive = false;
	};

}
