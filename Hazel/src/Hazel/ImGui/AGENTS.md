# ImGui — Dear ImGui 集成层

将 ImGui 作为 **Overlay Layer** 挂到 `Application`；负责上下文、主题、每帧 Begin/End 与默认帧缓冲清理。

上级索引：**[`../../../AGENTS.md`](../../../AGENTS.md)** · 根文档 §3

---

## 文件

| 文件 | 职责 |
|------|------|
| `ImGuiLayer.h/.cpp` | ImGui 初始化、每帧 `Begin`/`End`、深色主题 |

---

## 在 Application 中的位置

```
Application 构造
  → PushOverlay(m_ImGuiLayer)    // 栈顶 Overlay

每帧 Run():
  LayerStack::OnUpdate           // 含游戏 Layer（可能画到 FBO）
  ImGuiLayer::Begin()            // NewFrame
  LayerStack::OnImGuiRender()    // 各 Layer 画 ImGui 窗口
  ImGuiLayer::End()              // Clear 默认 FB → RenderDrawData
  Window::OnUpdate()             // SwapBuffers
```

**为何 End 里要 Clear：** 游戏/编辑器场景常画在**离屏 FBO**，默认帧缓冲不会被 Scene 清空；不 Clear 则拖动 ImGui 窗口会残留上一帧像素。

---

## 约束（vendor imgui docking + ImGuizmo）

- `ImGuiLayer` 启用 `DockingEnable` / `ViewportsEnable`；`End()` 中调用 `UpdatePlatformWindows` / `RenderPlatformWindowsDefault`。
- 字体路径：`assets/fonts/opensans/*.ttf`（构建时复制到 exe 旁）。
- `io.DisplaySize` 在 `End()` 中按窗口尺寸更新。

---

## 扩展方式

- 游戏/编辑器 UI：在自定义 `Layer::OnImGuiRender()` 里写 `ImGui::Begin/End`，**不要**改 ImGuiLayer 循环。
- 编辑器示例：`Hazelnut/EditorLayer.cpp`（Viewport、Stats、SceneHierarchy）。
