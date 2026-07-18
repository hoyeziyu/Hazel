# Platform — 平台与图形 API 实现

引擎抽象层的 **具体实现**：Windows 窗口/输入/对话框 + OpenGL 渲染后端。应用层与 `Hazel/Renderer` **不应**直接 `#include` 本目录（`Renderer::Submit` 中对 `OpenGLShader` 的 cast 为历史例外）。

上级索引：**[`../../AGENTS.md`](../../AGENTS.md)** · 根文档 §4

---

## 目录结构

```
Platform/
├── Windows/
│   ├── WindowsWindow.cpp      # GLFW 窗口、回调 → Event
│   ├── WindowsInput.cpp       # Input 静态 API
│   └── WindowsPlatformUtils.cpp  # 文件对话框
└── OpenGL/
    ├── OpenGLContext.cpp      # glad 加载
    ├── OpenGLRendererAPI.cpp  # Clear、Viewport、DrawIndexed
    ├── OpenGLShader.cpp
    ├── OpenGLBuffer.cpp
    ├── OpenGLVertexArray.cpp
    ├── OpenGLTexture.cpp      # STB_IMAGE_IMPLEMENTATION 唯一处
    └── OpenGLFramebuffer.cpp
```

---

## 与 Renderer 抽象的对应

| 抽象（Hazel/Renderer） | 平台实现 |
|------------------------|----------|
| `GraphicsContext::Create` | `OpenGLContext` |
| `RendererAPI` | `OpenGLRendererAPI` |
| `Shader::Create` | `OpenGLShader` |
| `VertexBuffer` / `IndexBuffer` | `OpenGLBuffer` |
| `VertexArray::Create` | `OpenGLVertexArray` |
| `Texture2D::Create` | `OpenGLTexture2D` |
| `Framebuffer::Create` | `OpenGLFramebuffer` |
| `Window::Create` | `WindowsWindow` |
| `Input::*` | `WindowsInput` |

工厂在 `Hazel/Renderer/*.cpp` 内 `switch (Renderer::GetAPI())` 分发到此。

---

## 启动与渲染流程

```
WindowsWindow::Init
  → glfwCreateWindow
  → GraphicsContext::Create → OpenGLContext::Init (glad)
  → 注册 GLFW 回调 → Application::OnEvent

OpenGLRendererAPI::Init
  → glEnable(BLEND)、DEPTH_TEST 等全局状态

DrawIndexed
  → glDrawElements（假定 VAO/Shader 已由上层 Bind）
```

---

## 修改时注意

- **OpenGL 头顺序：** `#include <glad/glad.h>` **先于** `#include <GLFW/glfw3.h>`。
- **stb：** 仅 `OpenGLTexture.cpp` 定义 `STB_IMAGE_IMPLEMENTATION`。
- **纹理合批查重：** `OpenGLTexture2D::operator==` 比较 `m_RendererID`。
- 当前仅 `HZ_PLATFORM_WINDOWS`；非 Windows 在 `Core.h` 报错。
- 新 OpenGL 类型：在 `Platform/OpenGL/` 实现 + 对应 `Hazel/Renderer` 工厂分支。
