# Events — 事件系统

同步、阻塞式事件：窗口/输入发生时立即构造 Event，经 `EventDispatcher` 分发给 Layer。

上级索引：**[`../../../AGENTS.md`](../../../AGENTS.md)** · 根文档 **[`../../../../AGENTS.md`](../../../../AGENTS.md)** §3

---

## 目录与职责

| 文件 | 职责 |
|------|------|
| `Event.h` | `Event` 基类、`EventType`、`EventCategory`、`EventDispatcher` |
| `ApplicationEvent.h` | `WindowResizeEvent`、`WindowCloseEvent` 等 |
| `KeyEvent.h` | `KeyPressedEvent`、`KeyReleasedEvent`、`KeyTypedEvent` |
| `MouseEvent.h` | `MouseMovedEvent`、`MouseButtonPressedEvent` 等 |

---

## 流程

```
GLFW 回调 (WindowsWindow.cpp)
  → 构造具体 Event
  → Application::OnEvent(e)
       → 内置处理 WindowClose / WindowResize
       → for (Layer 从顶到底)
            layer->OnEvent(e)
            if (e.Handled) break
```

**Layer 内典型用法：**

```cpp
EventDispatcher dispatcher(event);
dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(MyLayer::OnWindowResize));
```

- `Dispatch<T>`：若 `event.GetEventType() == T::GetStaticType()` 则调用 handler；handler 返回 `true` 表示已处理。
- `HZ_BIND_EVENT_FN` 定义于 `Core.h`，绑定成员函数 + `this`。

---

## 设计要点

- 事件**不缓冲**（注释中说明）；未来可改为 Event Bus。
- `Handled` 阻止继续向下传递。
- ImGuiLayer 可 `BlockEvents` 吞掉输入，避免传到游戏层。
- 新增事件：继承 `Event`，用 `EVENT_CLASS_TYPE` / `EVENT_CLASS_CATEGORY` 宏。
