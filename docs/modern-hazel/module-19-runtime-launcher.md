# M19 — Hazel-Runtime + Launcher

## 目标

在 M6 `AssetPack` / `RuntimeAssetManager` 基础上，提供**无 ImGui 的独立运行时**与**Launcher 启动器**，从打包资产加载 Start Scene 并进入主循环。

Editor Play 仍走 `CopyTo`；本模块面向 **Build Asset Pack 后的发布运行**。

---

## 核心模块

| 类型 | 路径 | 说明 |
|------|------|------|
| HDAT | `Project/ProjectRuntimeFormat.h` | `Project.hdat`：`StartSceneHandle` |
| 序列化 | `ProjectSerializer::SerializeRuntime/DeserializeRuntime` | Build 时写入，Runtime 启动时读取 |
| 资产路由 | `AssetManager.cpp` | `Project::IsRuntimeActive()` → `RuntimeAssetManager` |
| Build 集成 | `AssetPack::CreateFromActiveProject` | 同时写出 `assets/Project.hdat` |
| 运行时 | `Hazel-Runtime/` | `RuntimeLayer` + 无 ImGui `Application` |
| 启动器 | `Hazel-Launcher/` | ImGui Play → `CreateProcess` 启动 Runtime |
| 工具 | `tools/BuildSamplePack.cpp` | CLI 为 SampleProject 生成 `.hap` / `.hdat` |

---

## 运行时数据布局

```
MyGame/
├── Sample.hzproj          # 编辑器项目（Runtime 仍读取 Name、Script 路径等）
└── assets/
    ├── AssetPack.hap      # M6 烘焙包
    ├── Project.hdat       # HDAT：StartSceneHandle
    └── Scripts/Binaries/  # C# DLL（仍从磁盘加载，M19 不打包进 hap）
```

Pack 路径约定与 Editor **Build → Build Asset Pack** 一致：`{ProjectDir}/assets/AssetPack.hap`（不是项目根目录）。

---

## Hazel-Runtime 启动流程

```
CLI: Hazel-Runtime.exe [project-directory]
  默认 project-directory = exe 旁 SampleProject

1. 读取 *.hzproj
2. 读取 assets/Project.hdat → StartSceneHandle
3. AssetPack::Load(assets/AssetPack.hap)
4. Project::SetActiveRuntime(project, pack)
5. RuntimeAssetManager::LoadScene(StartSceneHandle)
6. Scene::OnRuntimeStart()
7. 每帧 OnUpdateRuntime + OnRenderRuntime（Primary Camera，无网格）
```

按键 **Esc** 关闭窗口。

`Application(..., enableImGui=false)` 跳过 `ImGuiLayer` 创建与 Run 循环中的 ImGui 阶段。

---

## Hazel-Launcher

- 默认项目路径：`SampleProject`（相对 exe 目录）
- **Play**：`CreateProcess` 启动同目录 `Hazel-Runtime.exe`，工作目录为 Launcher exe 目录
- **Browse**：选择 `.hzproj`，取其父目录作为项目根
- POST_BUILD 将 `Hazel-Runtime.exe`、`SampleProject`、`assets/shaders` 复制到 Launcher 输出目录

---

## CMake 选项

| 选项 | 默认 | 产物 |
|------|------|------|
| `HAZEL_BUILD_RUNTIME` | ON | `bin/.../Hazel-Runtime/Hazel-Runtime.exe` |
| `HAZEL_BUILD_LAUNCHER` | ON | `bin/.../Hazel-Launcher/Hazel-Launcher.exe` |
| `HAZEL_BUILD_SAMPLE_PACK_TOOL` | ON | `bin/.../Tools/BuildSamplePack.exe` |

---

## 构建与运行

```powershell
cmake --preset=windows-msvc-debug
cmake --build --preset=debug

# 首次：为 SampleProject 生成 Pack（需 OpenGL 上下文）
# 推荐在 Hazelnut 中使用 Build → Build Asset Pack
# 或使用 CLI 工具（在 repo 根目录运行）：
.\bin\Debug-windows-x86_64\Tools\BuildSamplePack.exe Hazelnut\SampleProject

# 直接运行（需 exe 旁已有 Pack）
.\bin\Debug-windows-x86_64\Hazel-Runtime\Hazel-Runtime.exe SampleProject

# 或通过 Launcher
.\bin\Debug-windows-x86_64\Hazel-Launcher\Hazel-Launcher.exe
```

启用 C# 时 Runtime/Launcher POST_BUILD 会部署 `DotNet/` 与 `Sample.dll`。

---

## 验收清单

- [x] Build Asset Pack 生成 `AssetPack.hap` + `Project.hdat`
- [x] `AssetManager::GetAsset` 在 Runtime 模式下从 Pack 加载
- [x] `Hazel-Runtime.exe SampleProject` 加载 Main 场景并渲染
- [x] Launcher Play 能启动 Runtime 进程
- [x] `ctest` 含 HDAT 与 Runtime 路由测试
- [x] Editor Play 从 Pack 加载（M22）

## 刻意未做

- Play 模式从 Pack 加载（Editor 仍 `CopyTo`）
- Pack 内嵌 C# DLL / App binary
- Tiering / ShaderPack
- 跨平台 Launcher（当前 Windows `CreateProcess`）

## 下一模块（M20）

SoundGraph / SoundBank（LD51 音频资产管线）。
