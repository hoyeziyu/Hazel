Hazel Game Engine

## 依赖管理

第三方库通过 **vcpkg manifest 模式** 管理，依赖声明在项目根目录的 `vcpkg.json` 中。
不需要对每个库单独执行 `git submodule add`。

目录约定：

```
vendor/
├── vcpkg/              # vcpkg 工具（git submodule）
└── vcpkg_installed/    # 已安装的包（自动生成，勿提交）
```

当前依赖：

- `spdlog` — 日志库
- `glfw3` — 窗口与输入
- `glad` — OpenGL 加载器
- `entt` — ECS 实体组件系统
- `glm` — 数学库
- `imgui` — 即时模式 GUI（含 glfw / opengl3 后端）
- `stb` — stb_image 等单头文件库
- `yaml-cpp` — YAML 序列化

均在 `vcpkg.json` 中声明，执行 `cmake --preset` 时自动安装。

## 首次设置

1. 在项目根目录拉取 vcpkg 子模块：

```
git submodule update --init vendor/vcpkg
```

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

首次执行 `cmake --preset` 时，vcpkg 会根据 `vcpkg.json` 自动安装 `spdlog` 等依赖。

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

构建产物目录（对齐 Premake）：

```
bin/<Config>-windows-x86_64/<ProjectName>/     # 最终 exe / lib / dll
bin-int/<Config>-windows-x86_64/<ProjectName>/ # 中间文件 .obj / .pdb
```
