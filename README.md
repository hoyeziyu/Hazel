Hazel Game Engine

## 依赖管理

第三方库主要通过 **vcpkg manifest**（`vcpkg.json`）管理；ImGui / ImGuizmo 为 **git submodule**（docking 版 UI）。

目录约定：

```
vendor/
├── vcpkg/              # vcpkg 工具（git submodule）
├── vcpkg_installed/    # 已安装的包（自动生成，勿提交）
├── imgui/              # ocornut/imgui docking 分支（git submodule）
└── ImGuizmo/           # Viewport Gizmo（git submodule）
```

ImGui / ImGuizmo 由 `cmake/HazelImGui.cmake` 编译，不在 vcpkg 中。

当前 vcpkg 依赖：

- `spdlog` — 日志库
- `glfw3` — 窗口与输入
- `glad` — OpenGL 加载器
- `entt` — ECS 实体组件系统
- `glm` — 数学库
- `stb` — stb_image 等单头文件库
- `yaml-cpp` — YAML 序列化
- `gtest` — Google Test 单元测试（`tests/HazelTests`）

均在 `vcpkg.json` 中声明，执行 `cmake --preset` 时自动安装。

Google Benchmark（`benchmarks/HazelBench`）通过 CMake FetchContent 获取，不在 `vcpkg.json` 中。

## 首次设置

1. 拉取 git submodules（vcpkg + ImGui + ImGuizmo）：

```
git submodule update --init --recursive
```

或仅初始化所需子模块：

```
git submodule update --init vendor/vcpkg vendor/imgui vendor/ImGuizmo
```

`vendor/imgui` 跟踪 **`docking`** 分支（见 `.gitmodules`）。

2. 初始化 vcpkg：

```
cd vendor/vcpkg
bootstrap-vcpkg.bat
cd ../..
```

3. 回到项目根目录配置和构建（不要在 `vendor/vcpkg` 里执行 cmake）：

```
cmake --preset=windows-msvc-debug
cmake --build --preset=debug
```

可选 CMake 选项（根 `CMakeLists.txt`）：

| 选项 | 默认 | 说明 |
|------|------|------|
| `HAZEL_BUILD_SANDBOX` | ON | 构建 Sandbox.exe |
| `HAZEL_BUILD_TESTS` | ON | 构建 Google Test（`HazelTests`） |
| `HAZEL_BUILD_BENCHMARKS` | ON | 构建 Google Benchmark（`HazelBench`） |

首次执行 `cmake --preset` 时，vcpkg 会根据 `vcpkg.json` 自动安装 `spdlog`、`gtest` 等依赖。

`CMAKE_TOOLCHAIN_FILE` 已在 `CMakePresets.json` 中配置为：
`vendor/vcpkg/scripts/buildsystems/vcpkg.cmake`

## 添加新依赖

1. 编辑 `vcpkg.json`，在 `dependencies` 中加入包名，例如：

```json
{
  "dependencies": [
    "spdlog",
    "glfw3"
  ]
}
```

2. 在对应 `CMakeLists.txt` 中链接：

```cmake
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
```

3. 重新配置项目：

```
cmake --preset=windows-msvc-debug
```

## 运行

```
.\bin\Debug-windows-x86_64\Sandbox\Sandbox.exe
```

## 单元测试（Google Test）

测试代码在 `tests/`，可执行文件 `HazelTests`，用于验证引擎模块（Scene、Components、序列化等）。

```powershell
# 构建
cmake --build --preset=debug --target HazelTests

# 运行全部测试（CMakePresets 已配置 testPresets）
ctest --preset debug --output-on-failure

# 或直接运行
.\bin\Debug-windows-x86_64\Tests\HazelTests.exe
```

新增测试：在 `tests/` 添加 `*Tests.cpp`，并注册到 `tests/CMakeLists.txt`。

## 性能基准（Google Benchmark）

基准代码在 `benchmarks/SceneBench.cpp`，可执行文件 `HazelBench`。与单元测试不同，**不走 ctest**，结果打印在终端。

```powershell
# 建议用 Release 测性能
cmake --preset=windows-msvc-release
cmake --build --preset=release --target HazelBench

.\bin\Release-windows-x86_64\Benchmarks\HazelBench.exe --benchmark_min_time=0.5s

# 只跑某一个 benchmark
.\bin\Release-windows-x86_64\Benchmarks\HazelBench.exe --benchmark_filter=BM_SceneCreateEntity

# 导出 JSON
.\bin\Release-windows-x86_64\Benchmarks\HazelBench.exe --benchmark_out=bench.json --benchmark_out_format=json
```

输出列含义：**CPU** 为每次迭代 CPU 时间（优先参考），**Time** 为墙钟时间，**Iterations** 为自动迭代次数。

## 构建产物目录

构建产物目录（对齐 Premake）：

```
bin/<Config>-windows-x86_64/<ProjectName>/     # 最终 exe / lib / dll
bin-int/<Config>-windows-x86_64/<ProjectName>/ # 中间文件 .obj / .pdb
```

常见产物：

| 路径 | 说明 |
|------|------|
| `bin/.../Sandbox/Sandbox.exe` | 沙盒应用 |
| `bin/.../Hazel/hazel-engine.lib` | 引擎静态库 |
| `bin/.../Tests/HazelTests.exe` | 单元测试 |
| `bin/.../Benchmarks/HazelBench.exe` | 性能基准 |
