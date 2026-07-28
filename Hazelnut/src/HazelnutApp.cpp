#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>
#include "EditorLayer.h"


namespace Hazel {
	class HazelEditor : public Application
	{
	public:
		HazelEditor() : Application("Hazel Editor")
		{
			PushLayer(new EditorLayer());
		}
		~HazelEditor()
		{
		}
	};
	Application* CreateApplication(int argc, char** argv)
	{
		(void)argc;
		(void)argv;
		return new HazelEditor();
	}
}