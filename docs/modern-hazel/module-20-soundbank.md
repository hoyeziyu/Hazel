# M20 — SoundBank / SoundConfig（Phase A）

> 对照：`HazelEngine/Hazel/src/Hazel/Audio/SoundBank.*`、`SoundObject.h`（SoundConfig）  
> LD51 示例：`HazelGame/LD51-main/Assets/Audio/Configs/SC_Move.hsoundc`

M20 Phase A 实现 **SoundConfig 资产 + SoundBank 打包 + Play 前构建**，不包含 SoundGraph 节点图（Phase B）。

---

## 资产类型

| 扩展名 | `AssetType` | 说明 |
|--------|-------------|------|
| `.wav` | `Audio` | 原始音频，`AudioFile` 记录 `FileSize` |
| `.hsoundc` | `SoundConfig` | YAML 配置，引用 `AssetID`（wav handle） |
| `.hsb` | — | 运行时 SoundBank 二进制包（非注册表资产） |

SoundConfig YAML 兼容 LD51 扁平原格式：

```yaml
AssetID: 20402389245123987104
IsLooping: false
VolumeMultiplier: 0.8
PitchMultiplier: 1.0
```

---

## SoundBank（`.hsb`）

- 魔数 `HZSB`，版本 `2`
- 布局：Header → Index 表（handle + offset/size/info）→ 交错 wav 字节
- `SoundBank::Create(waveHandles, path)` — 编辑器 Build 时写入
- `SoundBank::Load(path)` — Play / Runtime 加载
- `AudioEngine::BuildSoundBank` — 扫描项目中所有 `SoundConfig`，收集其 `DataSourceAsset`（`.wav`）

输出路径：`<project>/assets/SoundBank.hsb`

---

## AudioEngine 扩展

| API | 行为 |
|-----|------|
| `PlaySound(path)` | M17 磁盘路径播放 |
| `PlaySound(AssetHandle)` | 优先从 SoundBank 内存解码，否则回退磁盘 |
| `PlaySoundConfig(AssetHandle)` | 读 `SoundConfigAsset`，应用 Volume/Loop |
| `BuildSoundBank(path)` | 打包 wav → `.hsb` |
| `LoadSoundBank(path)` | 加载已构建 bank |

内存播放使用 `ma_decoder_init_memory` + `ma_sound_init_from_data_source`。

---

## AudioComponent

```cpp
AssetHandle SoundConfig = 0;  // 优先
std::string FilePath;         // M17 遗留回退
```

`Scene::OnRuntimeStart`：`SoundConfig` 非 0 时走 `PlaySoundConfig`，否则用 `FilePath`。

---

## 编辑器集成

- **Build → Build Sound Bank** — 手动打包
- **Play** — `OnScenePlay()` 前自动 `BuildSoundBank` + `LoadSoundBank`
- **Scene Hierarchy → Audio** — SoundConfig 下拉选择

---

## SampleProject

| 路径 | Handle |
|------|--------|
| `assets/Audio/Source/click.wav` | `20402389245123987104` |
| `assets/Audio/Configs/SC_Click.hsoundc` | `21402389245123987105` |

`Main.hazel` 含 **Click Audio** 实体，`PlayOnAwake` 引用 `SC_Click`。

验证：打开 SampleProject → Play，应听到短促 click 音（需 `HZ_AUDIO` / miniaudio）。

---

## 测试

`tests/AudioTests.cpp`：

- Engine init/shutdown
- AudioComponent YAML（含 SoundConfig）
- SoundConfig YAML 往返
- `AudioFileUtils` WAV 头解析
- 空 SoundBank 加载

---

## Phase B（未做）

- `.sound_graph` SoundGraph 节点图
- 3D 空间化、Audio Command Registry
- Runtime 强制 SoundBank 校验

---

## 相关模块

- M17 miniaudio 基础 — [module-17-miniaudio.md](./module-17-miniaudio.md)
- M19 Runtime — Play 模式与 Runtime 可共用 SoundBank 路径约定
