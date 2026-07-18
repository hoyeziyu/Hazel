# Core — 应用运行时核心

应用生命周期、窗口、Layer 栈、日志、输入抽象与全局宏。客户端（Sandbox / Hazelnut）通过 Layer 驱动游戏逻辑。

上级索引：**[`../../../AGENTS.md`](../../../AGENTS.md)**（引擎库）· 根文档 **[`../../../../AGENTS.md`](../../../../AGENTS.md)** §3

---

## 目录与职责

| 文件 | 职责 |
|------|------|
| `Application.h/.cpp` | 主循环、`LayerStack`、挂载 `ImGuiLayer`、窗口事件入口 |
| `Layer.h/.cpp` | 可扩展层：`OnAttach/OnUpdate/OnImGuiRender/OnEvent` |
| `LayerStack.h/.cpp` | Layer 顺序；Overlay 插在末尾（如 ImGui） |
| `Window.h/.cpp` | 窗口抽象；工厂 `Window::Create()` |
| `EntryPoint.h` | `main` → `Log::Init` → `CreateApplication()` → `Run()` |
| `Log.h/.cpp` | spdlog 封装：`HZ_CORE_*` / `HZ_*` |
| `Core.h` | `Ref`/`Scope`、`HZ_ASSERT`、`HZ_BIND_EVENT_FN`、平台宏 |
| `Timestep.h` | 帧间隔 `float`（秒） |
| `Input.h` | 静态输入 API；平台实现在 `Platform/Windows/WindowsInput` |
| `KeyCodes.h` / `MouseCodes.h` | 键鼠常量 |

---

## 主循环流程

```
main (EntryPoint.h)
  → Log::Init()
  → CreateApplication()          // 客户端实现，PushLayer(ExampleLayer) 等
  → Application::Run()
       while (m_Running)
         ├─ LayerStack::OnUpdate(timestep)     // 从底到顶
         ├─ ImGuiLayer::Begin()
         ├─ LayerStack::OnImGuiRender()
         ├─ ImGuiLayer::End()                 // Clear 默认 FB + 画 ImGui
         └─ Window::OnUpdate()                // glfwPollEvents + SwapBuffers
```

**事件路径：** GLFW 回调 → `Application::OnEvent` → 从**栈顶 Layer 向下**分发，直到 `e.Handled == true`。

**Layer vs Overlay：** `PushLayer` 插入 `m_LayerInsertIndex` 之前（游戏层）；`PushOverlay` 追加到末尾（ImGui 始终在上）。

---

## 修改时注意

- 新增游戏逻辑：在**客户端**继承 `Layer`，不要改 `Application` 循环除非必要。
- `CreateApplication()` 只在 Sandbox / Hazelnut 的 `*App.cpp` 中定义。
- 日志严重级别用 **`critical()`**，不要用 `fatal()`。
- 非 Windows 平台当前 `#error`（见 `Core.h`）。
