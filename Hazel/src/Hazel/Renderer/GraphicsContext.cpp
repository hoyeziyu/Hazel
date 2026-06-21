#include "hzpch.h"
#include "Hazel/Renderer/GraphicsContext.h"

#include "Platform/OpenGL/OpenGLContext.h"

namespace Hazel {

	Scope<GraphicsContext> GraphicsContext::Create(GLFWwindow* windowHandle)
	{
		return CreateScope<OpenGLContext>(windowHandle);
	}

}
