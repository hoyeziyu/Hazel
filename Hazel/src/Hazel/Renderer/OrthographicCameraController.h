#pragma once


#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/MouseEvent.h"


namespace Hazel {

	struct OrthographicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};

	// OrthographicCamera 的包装器
	class OrthographicCameraController
	{

	public:
		// 默认情况： 纵横比（eg: 16:9） * 2 units
		OrthographicCameraController(float aspectRatio, bool rotation = false);
		void OnUpdate(Timestep ts);		// 时间步长
		void OnEvent(Event& e);
		void OnResize(float width, float height);
		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }
		const OrthographicCameraBounds& GetBounds() const { return m_Bounds; }
		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; CalculateView();}

	private:
		void CalculateView();
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	
	private:
		float m_AspectRatio;		// 纵横比
		float m_ZoomLevel = 1.0f;	// 实际缩放等级
		OrthographicCameraBounds m_Bounds;	// 这里要注意 bound 和 camera的初始化顺序；不然会出现第一帧黑屏情况
		OrthographicCamera m_Camera;
		bool m_Rotation;
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
	};
}