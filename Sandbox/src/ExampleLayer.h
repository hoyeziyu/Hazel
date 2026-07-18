#pragma once

#include "Hazel/Core/Layer.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Scene/Scene.h"

// ExampleLayer：演示「Layer 驱动 Scene」的最小用法（A0）。
//
// Layer 与 Scene 的关系（必读）：
//   Application
//     └─ LayerStack
//          └─ ExampleLayer          ← 生命周期 / 事件 / 清屏（应用层）
//               └─ m_Scene (Ref)    ← 实体、相机、Sprite（世界层）
//                    └─ Renderer2D  ← 真正 DrawCall
//
// Layer 不是渲染「图层混合」；它是引擎把 Update/Event 切片的方式。
// Scene 不继承 Layer，也不注册到 LayerStack；必须由 Layer（或其它宿主）持有并每帧调用。
class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer();
	~ExampleLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Hazel::Timestep ts) override;
	void OnEvent(Hazel::Event& event) override;

private:
	bool OnWindowResize(Hazel::WindowResizeEvent& e);

	// 本 Layer 拥有的世界；销毁 Layer 时在 OnDetach 里 reset。
	Hazel::Ref<Hazel::Scene> m_Scene;
};
