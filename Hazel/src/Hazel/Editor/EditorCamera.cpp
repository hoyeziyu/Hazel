#include "hzpch.h"
#include "EditorCamera.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/MouseCodes.h"
#include "Hazel/Events/MouseEvent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>

namespace Hazel {

	EditorCamera::EditorCamera(float verticalFovDeg, float width, float height, float nearClip, float farClip)
		: m_VerticalFovDeg(verticalFovDeg), m_NearClip(nearClip), m_FarClip(farClip)
	{
		m_ViewportWidth = (uint32_t)width;
		m_ViewportHeight = (uint32_t)height;
		m_AspectRatio = width / height;

		m_FocalPoint = glm::vec3(0.0f);
		m_Position = glm::vec3(-5.0f, 5.0f, 5.0f);
		m_Distance = glm::distance(m_Position, m_FocalPoint);
		m_Yaw = 3.0f * glm::pi<float>() / 4.0f;
		m_Pitch = glm::pi<float>() / 4.0f;

		UpdateProjection();
		UpdateView();
	}

	void EditorCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			return;
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;
		m_AspectRatio = (float)width / (float)height;
		UpdateProjection();
	}

	void EditorCamera::OnUpdate(Timestep ts)
	{
		(void)ts;

		if (!m_IsActive)
			return;

		const glm::vec2 mouse = { Input::GetMouseX(), Input::GetMouseY() };
		const glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.002f;
		m_InitialMousePosition = mouse;

		const bool alt = Input::IsKeyPressed(HZ_KEY_LEFT_ALT) || Input::IsKeyPressed(HZ_KEY_RIGHT_ALT);
		if (alt)
		{
			if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_MIDDLE))
				MousePan(delta);
			else if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_LEFT))
				MouseRotate(delta);
			else if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_RIGHT))
				MouseZoom((delta.x + delta.y) * 0.1f);
		}

		m_Yaw += m_YawDelta;
		m_Pitch += m_PitchDelta;
		m_Position = CalculatePosition() + m_PositionDelta;

		UpdateView();

		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_PositionDelta *= 0.8f;
	}

	void EditorCamera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		if (!m_IsActive)
			return false;

		MouseZoom(e.GetYOffset() * 0.1f);
		UpdateView();
		return false;
	}

	void EditorCamera::UpdateProjection()
	{
		m_Projection = glm::perspective(glm::radians(m_VerticalFovDeg), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		const float yawSign = GetUpDirection().y < 0.0f ? -1.0f : 1.0f;
		const glm::vec3 lookAt = m_Position + GetForwardDirection();
		m_ViewMatrix = glm::lookAt(m_Position, lookAt, glm::vec3(0.0f, yawSign, 0.0f));
		m_Distance = glm::distance(m_Position, m_FocalPoint);
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint -= GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		const float yawSign = GetUpDirection().y < 0.0f ? -1.0f : 1.0f;
		m_YawDelta += yawSign * delta.x * RotationSpeed();
		m_PitchDelta += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
			m_Distance = 1.0f;

		m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch - m_PitchDelta, -m_Yaw - m_YawDelta, 0.0f));
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::mat3_cast(GetOrientation()) * glm::vec3(0.0f, 1.0f, 0.0f);
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::mat3_cast(GetOrientation()) * glm::vec3(1.0f, 0.0f, 0.0f);
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::mat3_cast(GetOrientation()) * glm::vec3(0.0f, 0.0f, -1.0f);
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		const float x = std::min((float)m_ViewportWidth / 1000.0f, 2.4f);
		const float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		const float y = std::min((float)m_ViewportHeight / 1000.0f, 2.4f);
		const float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.3f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		return std::min(speed, 50.0f);
	}

}
