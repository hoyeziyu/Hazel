# Hazel 引擎库

本目录为 **hazel-engine** 静态库（CMake 别名 `Hazel::Engine`）。

完整结构、模块说明、平台层与构建约定见仓库根目录 **[`../AGENTS.md`](../AGENTS.md)**，重点章节：

- **§3** — `src/Hazel/` 各模块（Core / Events / Renderer / Scene / ImGui / Debug / Utils）
- **§4** — `src/Platform/`（Windows + OpenGL）
- **§9** — 修改引擎代码时的注意事项

## 本目录要点

```
Hazel/
├── CMakeLists.txt          # GLOB_RECURSE src/*.cpp|*.h，PCH: hzpch.h
└── src/
    ├── Hazel.h             # 应用侧入口头
    ├── hzpch.h
    ├── Hazel/              # 引擎逻辑
    └── Platform/           # 平台实现
```

- Include 根：`Hazel/src`（公开 API 以 `Hazel/...` 路径包含）
- 输出：`bin/<Config>-windows-x86_64/Hazel/hazel-engine.lib`
