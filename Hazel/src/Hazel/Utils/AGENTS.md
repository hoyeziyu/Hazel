# Utils — 平台工具

与渲染/ECS 无关的 OS 级辅助；当前主要是原生文件对话框。

上级索引：**[`../../../AGENTS.md`](../../../AGENTS.md)** · 平台实现 **[`../../Platform/AGENTS.md`](../../Platform/AGENTS.md)**

---

## 文件

| 文件 | 职责 |
|------|------|
| `PlatformUtils.h` | 声明 `FileDialogs::OpenFile` / `SaveFile` |

---

## 流程

```
EditorLayer（或客户端）
  → FileDialogs::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0")
  → WindowsPlatformUtils 内 Win32 GetOpenFileName / GetSaveFileName
  → std::optional<std::string> 路径
  → SceneSerializer::Deserialize / Serialize
```

---

## 修改时注意

- API 返回 `std::optional`；取消对话框时为空。
- 过滤器字符串为 Win32 格式：`"描述\0*.ext\0"`。
- 仅 Windows 实现；其它平台需新增 `Platform/*` 实现后再扩展工厂。
