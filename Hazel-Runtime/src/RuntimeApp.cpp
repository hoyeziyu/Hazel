#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>
#include "RuntimeLayer.h"

class RuntimeApplication : public Hazel::Application
{
public:
	RuntimeApplication(int argc, char** argv)
		: Application("Hazel Runtime", false)
	{
		PushLayer(new Hazel::RuntimeLayer(argc, argv));
	}
};

Hazel::Application* Hazel::CreateApplication(int argc, char** argv)
{
	return new RuntimeApplication(argc, argv);
}
