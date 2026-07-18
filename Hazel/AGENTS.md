# Hazel 引擎库

本目录为 **hazel-engine** 静态库（CMake 别名 `Hazel::Engine`）。

完整结构、构建约定见仓库根目录 **[`../AGENTS.md`](../AGENTS.md)**。

---

## 源码布局

```
Hazel/
├── CMakeLists.txt
└── src/
    ├── Hazel.h / hzpch.h / hzpch.cpp
    ├── Hazel/                 ← 引擎逻辑（各子模块见下表）
    └── Platform/              ← Windows + OpenGL 实现
```

Include 根：`Hazel/src`（公开 API：`#include "Hazel/..."`）  
产物：`bin/<Config>-windows-x86_64/Hazel/hazel-engine.lib`

---

## 模块文档（作用 + 流程）

| 模块 | 路径 | AGENTS.md |
|------|------|-----------|
| **Core** | `src/Hazel/Core/` | [Core/AGENTS.md](src/Hazel/Core/AGENTS.md) — Application 主循环、Layer、Window、Log |
| **Events** | `src/Hazel/Events/` | [Events/AGENTS.md](src/Hazel/Events/AGENTS.md) — 同步事件分发 |
| **Renderer** | `src/Hazel/Renderer/` | [Renderer/AGENTS.md](src/Hazel/Renderer/AGENTS.md) — 渲染抽象、Renderer2D 合批 |
| **Scene** | `src/Hazel/Scene/` | [Scene/AGENTS.md](src/Hazel/Scene/AGENTS.md) — EnTT ECS、序列化 |
| **ImGui** | `src/Hazel/ImGui/` | [ImGui/AGENTS.md](src/Hazel/ImGui/AGENTS.md) — ImGui Overlay 集成 |
| **Debug** | `src/Hazel/Debug/` | [Debug/AGENTS.md](src/Hazel/Debug/AGENTS.md) — Chrome trace profiling |
| **Utils** | `src/Hazel/Utils/` | [Utils/AGENTS.md](src/Hazel/Utils/AGENTS.md) — 文件对话框等 |
| **Platform** | `src/Platform/` | [Platform/AGENTS.md](src/Platform/AGENTS.md) — OpenGL + Windows 实现 |

---

## 端到端一帧（编辑器典型）

```
Application::Run
  → EditorLayer::OnUpdate
       Framebuffer::Bind → Clear → Scene::OnUpdate → Renderer2D 合批 → Unbind
  → ImGuiLayer::Begin
  → EditorLayer::OnImGuiRender（Hierarchy / Stats / Viewport 显示 FBO）
  → ImGuiLayer::End（Clear 默认 FB + 画 ImGui）
  → SwapBuffers
```

客户端示例见 **[`../Sandbox/AGENTS.md`](../Sandbox/AGENTS.md)**、**[`../Hazelnut/AGENTS.md`](../Hazelnut/AGENTS.md)**。

---

## 修改引擎时注意

- 新 `Hazel/src/**/*.cpp` 由 GLOB 自动纳入 CMake；**Sandbox/Hazelnut 新 cpp 须手动加入 CMakeLists**。
- PCH：`hzpch.h`；引擎 `.cpp` 首行 `#include "hzpch.h"`。
- 遇 MSVC C1041 / PCH 占用：结束残留 `cl.exe` 后重试。
