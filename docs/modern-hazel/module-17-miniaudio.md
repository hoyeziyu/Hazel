# M17 — miniaudio 基础音频

## 目标

接入 [miniaudio](https://github.com/mackron/miniaudio)（header-only），支持 Play 模式下播放 WAV 等常见格式音效，为 LD50/LD51 类游戏提供基础音频能力。

## 依赖

- vcpkg 包 `miniaudio`
- CMake 选项 `HZ_AUDIO`（默认 ON）

## 核心模块

| 类型 | 路径 | 说明 |
|------|------|------|
| 引擎 | `Hazel/Audio/AudioEngine.h/.cpp` | `ma_engine` 初始化、播放、停止、自动清理已结束音效 |
| 组件 | `AudioComponent` | `FilePath`、`Volume`、`PlayOnAwake`、`Loop` |
| 生命周期 | `Scene::OnRuntimeStart/Stop` | Play 时按组件播放，Stop 时停止 |
| 序列化 | `SceneSerializer` | YAML 读写 `AudioComponent` |

## 资源路径

- 组件 `FilePath` 为相对 **项目 assets 目录** 的路径，例如 `Audio/click.wav`
- 示例文件：`Hazelnut/SampleProject/assets/Audio/click.wav`
- 构建 Sandbox/Hazelnut 时 POST_BUILD 会复制 `assets/` 到 exe 旁

## 编辑器用法

1. 打开 SampleProject
2. 选中实体 → Add Component → **Audio**
3. File Path 填 `Audio/click.wav`
4. 勾选 Play On Awake / Loop、调节 Volume
5. Play 验证播放；Stop 后音效停止

## API 示例（C++）

```cpp
AudioEngine::Get().Init();
uint32_t handle = AudioEngine::Get().PlaySound(path, 1.0f, false);
AudioEngine::Get().StopSound(handle);
AudioEngine::Get().Update(); // 每帧清理已结束的 one-shot
AudioEngine::Get().Shutdown();
```

## 自测清单

```powershell
cmake --preset=windows-msvc-debug
cmake --build --preset=debug --target Hazelnut
ctest --preset=debug
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe
```

- [x] 启动日志：`[Audio] miniaudio engine initialized`
- [x] Audio 组件可序列化到 YAML
- [x] Play 模式播放 `Audio/click.wav`
- [x] ctest 含 `AudioTest.*`

## 后续扩展（M20 等）

- 3D 空间音效 / Listener
- SoundBank、SoundGraph
- C# `Audio.Play()` InternalCall
- OGG/MP3 资源管线与 Asset 类型
