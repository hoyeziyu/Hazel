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

## 单元测试（Google Test）

引擎逻辑的单测在仓库根目录 **`tests/`**（不在 Sandbox 内），链接 `Hazel::Engine`，由 vcpkg 提供 `gtest`。

| 项 | 说明 |
|----|------|
| 目标 | `HazelTests` |
| 产物 | `bin/<Config>-windows-x86_64/Tests/HazelTests.exe` |
| CMake 开关 | `HAZEL_BUILD_TESTS`（默认 ON） |

```powershell
cmake --build --preset=debug --target HazelTests
ctest --preset debug --output-on-failure
# 或直接运行
.\bin\Debug-windows-x86_64\Tests\HazelTests.exe
```

新增用例：在 `tests/` 下添加 `*Tests.cpp`，并写入 `tests/CMakeLists.txt` 的 `add_executable(HazelTests ...)`。

**测什么：** Scene / Entity / Components、序列化、数学等纯逻辑。**不要**在无 OpenGL 上下文时调用 `Renderer::Init()`、`Renderer2D::BeginScene()` 等。

## 性能基准（Google Benchmark）

基准在仓库根目录 **`benchmarks/`**，目标 `HazelBench`，通过 FetchContent 拉取 google/benchmark（不在 `vcpkg.json`）。

| 项 | 说明 |
|----|------|
| 目标 | `HazelBench` |
| 产物 | `bin/<Config>-windows-x86_64/Benchmarks/HazelBench.exe` |
| CMake 开关 | `HAZEL_BUILD_BENCHMARKS`（默认 ON） |

```powershell
cmake --build --preset=release --target HazelBench
.\bin\Release-windows-x86_64\Benchmarks\HazelBench.exe --benchmark_min_time=0.5s
```

Benchmark **不走** `ctest`，结果以终端表格输出（Time / CPU / Iterations）。性能对比请用 **Release** 构建。

新增基准：在 `benchmarks/SceneBench.cpp`（或新 `.cpp`）中写 `static void BM_...(benchmark::State& state)` 并 `BENCHMARK(BM_...)`，若新增源文件需改 `benchmarks/CMakeLists.txt`。

## 与 Sandbox 的关系

- Sandbox 是**可运行的示例客户端**；`tests/` 与 `benchmarks/` 是**引擎质量与性能工具**，三者均链接同一套 `Hazel::Engine`。
- 改引擎 Scene / Renderer 等模块后，建议先跑 `HazelTests`，需要时再用 `HazelBench` 看性能变化。
