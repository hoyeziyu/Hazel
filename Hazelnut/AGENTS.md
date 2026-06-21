# Hazelnut（编辑器 — 预留）

对应 Cherno 教程后期的 **Hazel 编辑器** 项目。

## 当前状态

- 目录存在，**尚无源码与 CMake 目标**
- 根 `CMakeLists.txt` 未 `add_subdirectory(Hazelnut)`

## 接入时建议

1. 参照 `Sandbox/`：`CMakeLists.txt` + `src/*App.cpp` + 链接 `Hazel::Engine`
2. 在根 `CMakeLists.txt` 增加 `option(HAZEL_BUILD_HAZELNUT ...)` 与 `add_subdirectory(Hazelnut)`
3. 编辑器 UI 可复用引擎内 `ImGuiLayer` 与 Scene 系统

详见根目录 **[`../AGENTS.md`](../AGENTS.md)** §6。
