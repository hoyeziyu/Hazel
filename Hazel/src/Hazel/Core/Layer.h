#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Core/Timestep.h"

namespace Hazel {
	// Layer = 应用「逻辑切片」，由 Application 每帧按 LayerStack 顺序调用。
	// 它不拥有「世界里有哪些实体」的概念；常见做法是 Layer 持有/驱动一个 Scene。
	// 对比：Scene = 游戏世界（ECS）；Layer = 何时更新、谁响应事件、是否清屏。
	class HAZEL_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;
		virtual void OnAttach() {}   // PushLayer 后立刻调用：适合创建 Scene、加载资源
		virtual void OnDetach() {}   // 弹出层时：释放 Scene 等
		virtual void OnUpdate([[maybe_unused]] Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent([[maybe_unused]] Event& event) {}
		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}