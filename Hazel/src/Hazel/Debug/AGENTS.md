# Debug — 性能分析

Chrome Tracing 兼容的 instrumentation：`HZ_PROFILE_*` 宏，输出 JSON 到 `./log/`。

上级索引：**[`../../../AGENTS.md`](../../../AGENTS.md)** · 根文档 §3

---

## 文件

| 文件 | 职责 |
|------|------|
| `Instrumentor.h` | `Instrumentor`、`InstrumentationTimer`、`HZ_PROFILE_*` 宏 |

---

## 宏用法

| 宏 | 作用 |
|----|------|
| `HZ_PROFILE_BEGIN_SESSION(name, filepath)` | 开始记录会话 |
| `HZ_PROFILE_END_SESSION()` | 结束并写 JSON |
| `HZ_PROFILE_FUNCTION()` | 自动以函数签名为 scope 名 |
| `HZ_PROFILE_SCOPE(name)` | 手动命名 scope |

**开关：** `Instrumentor.h` 内 `#define HZ_PROFILE 0`；改为 `1` 启用。为 `0` 时宏展开为空，零开销。

---

## 流程（EntryPoint.h）

```
Startup  session  → CreateApplication()
Runtime  session  → app->Run()
Shutdown session  → delete app
```

输出示例：`./log/HazelProfile-Runtime.json`，用 Chrome `chrome://tracing` 或 Perfetto 查看。

---

## 修改时注意

- 使用宏的文件须能见到 `Instrumentor.h`（引擎 `.cpp` 经 `hzpch.h`；Hazelnut 等客户端需**显式 include**）。
- `HZ_PROFILE_FUNCTION()` 在 `Renderer2D.cpp`、`EditorLayer.cpp` 等热点路径广泛使用。
