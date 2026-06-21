# Sandbox 应用

链接 `Hazel::Engine` 的客户端 exe，用于实验与教程示例。

完整说明见仓库根目录 **[`../AGENTS.md`](../AGENTS.md)** §5。

## 本目录结构

```
Sandbox/
├── CMakeLists.txt       # SANDBOX_SOURCES、POST_BUILD 复制 assets
├── src/
│   └── SandboxApp.cpp   # 实现 Hazel::CreateApplication()
└── assets/              # 运行时资源（相对 exe 路径）
    ├── shaders/         # Texture.glsl, FlatColor.glsl
    ├── fonts/opensans/  # ImGui 字体
    ├── textures/        # 纹理（大文件可能在 .gitignore）
    └── game/
```

## 要点

- 构建产物：`bin/<Config>-windows-x86_64/Sandbox/Sandbox.exe`
- 资源在 build 时复制到 exe 旁的 `assets/`；路径如 `assets/shaders/Texture.glsl`
- 新增 `.cpp` 须写入 `CMakeLists.txt` 的 `SANDBOX_SOURCES`
- 默认 Sandbox 无渲染逻辑，黑屏正常；profile 日志输出到 `./log/`
