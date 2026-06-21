#pragma once

#include "Hazel/Core/Core.h"

struct GLFWwindow;

namespace Hazel {
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;

		static Scope<GraphicsContext> Create(GLFWwindow* windowHandle);
	};
}