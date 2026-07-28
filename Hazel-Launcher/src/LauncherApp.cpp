#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>
#include "LauncherLayer.h"

class LauncherApplication : public Hazel::Application
{
public:
	LauncherApplication()
		: Application("Hazel Launcher")
	{
		PushLayer(new Hazel::LauncherLayer());
	}
};

Hazel::Application* Hazel::CreateApplication(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	return new LauncherApplication();
}
