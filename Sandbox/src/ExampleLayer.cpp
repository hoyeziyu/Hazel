#include "ExampleLayer.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Components.h"

ExampleLayer::ExampleLayer()
	: Layer("ExampleLayer")
{
}

void ExampleLayer::OnAttach()
{
	// Layer 负责「创建并配置」Scene；世界内容放在 Scene 里，不放在 Layer 基类里。
	m_Scene = Hazel::CreateRef<Hazel::Scene>();

	// 先设视口，再加 Camera：SceneCamera 需要宽高比，否则投影矩阵无效。
	auto& window = Hazel::Application::Get().GetWindow();
	m_Scene->OnViewportResize(window.GetWidth(), window.GetHeight());

	// 可渲染实体：Transform（CreateEntity 已加）+ SpriteRenderer
	Hazel::Entity square = m_Scene->CreateEntity("Square");
	square.AddComponent<Hazel::SpriteRendererComponent>(glm::vec4{ 0.8f, 0.2f, 0.3f, 1.0f });

	// 主相机：Scene::OnUpdate 只在存在 Primary Camera 时才会 BeginScene/Draw
	Hazel::Entity camera = m_Scene->CreateEntity("Camera");
	auto& cameraComponent = camera.AddComponent<Hazel::CameraComponent>();
	cameraComponent.Primary = true;

	HZ_INFO("ExampleLayer attached: Square + Camera created");
}

void ExampleLayer::OnDetach()
{
	m_Scene.reset();
}

void ExampleLayer::OnUpdate(Hazel::Timestep ts)
{
	// 清屏属于「这一层这一帧怎么呈现」，由 Layer 做；Scene 只负责世界更新与提交绘制。
	Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	Hazel::RenderCommand::Clear();

	// 把帧时间交给 Scene：脚本 → 选相机 → Renderer2D 批处理 Sprite
	m_Scene->OnUpdate(ts);
}

void ExampleLayer::OnEvent(Hazel::Event& event)
{
	// 事件先到 Layer，再按需转给 Scene（例如改视口）。Scene 自己不挂在事件总线中。
	Hazel::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Hazel::WindowResizeEvent>(HZ_BIND_EVENT_FN(ExampleLayer::OnWindowResize));
}

bool ExampleLayer::OnWindowResize(Hazel::WindowResizeEvent& e)
{
	if (e.GetWidth() == 0 || e.GetHeight() == 0)
		return false;

	m_Scene->OnViewportResize(e.GetWidth(), e.GetHeight());
	return false; // false = 不吞掉事件，其它 Layer 仍可收到
}
