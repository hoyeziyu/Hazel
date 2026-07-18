#include <Hazel.h>
#include "ExampleLayer.h"

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		// PushLayer → LayerStack 管理，并立刻 OnAttach。
		// ExampleLayer 在内部创建 Scene；Application 只认识 Layer，不直接认识 Scene。
		PushLayer(new ExampleLayer());
	}

	~Sandbox() = default;
};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}
