# M21 — Runtime / Pack / SoundBank 闭环

> 承接 M19 Runtime 启动链与 M20 SoundBank，补齐 **Build → Pack → Runtime 加载** 的完整音频与资产路径。

---

## 目标

| 项 | 说明 |
|----|------|
| Runtime 启动 | `Hazel-Runtime` 加载 `assets/SoundBank.hsb` |
| Build Asset Pack | `AssetPack::CreateFromActiveProject` 顺带 `BuildSoundBank` |
| SampleProject 预置 | 提交 `AssetPack.hap` + `Project.hdat` + `SoundBank.hsb` |
| 测试 | `RuntimeTests` 不再 `GTEST_SKIP`，校验 SoundConfig / SoundBank |
| 大整数 Handle | Registry / Scene / SoundConfig YAML 使用 `strtoull` 行解析，避免 yaml-cpp 溢出 |

---

## 运行时布局（M21 后）

```
Hazelnut/SampleProject/assets/
├── AssetRegistry.hzr
├── AssetPack.hap       # Build 产物，已预置
├── Project.hdat        # StartSceneHandle
├── SoundBank.hsb       # click.wav 打包
├── Audio/
│   ├── Source/click.wav
│   └── Configs/SC_Click.hsoundc
└── scenes/Main.hazel
```

---

## 代码改动摘要

### AssetPack 构建

`AssetPack::CreateFromActiveProject` 在写出 `Project.hdat` 后调用 `AudioEngine::BuildSoundBank`，再序列化 `.hap`。场景资产列表会递归收集 `SoundConfig → DataSourceAsset`（wav）。

### Hazel-Runtime

`RuntimeLayer` 在 `SetActiveRuntime` 之后、`LoadScene` 之前：

```cpp
const auto soundBankPath = project->GetAssetDirectory() / "SoundBank.hsb";
if (std::filesystem::exists(soundBankPath))
    AudioEngine::Get().LoadSoundBank(soundBankPath);
```

### BuildSamplePack 工具

- `Log::InitHeadless()` — CLI 无 stdout 阻塞
- exe 目录为 cwd，POST_BUILD 复制 Sandbox `assets/shaders`
- 仅在 `SoundBank.hsb` 实际存在时打印成功

### EditorAssetManager

- `EditorAssetManager(false)`（`SetActiveEditor`）不扫描磁盘、不写回 registry
- `LoadAssetRegistry` 行解析 + `strtoull`，避免 yaml-cpp 大整数损坏 Handle

### SoundConfig 反序列化

`AssetID` 从原始 YAML 文本提取（`ParseUint64FieldFromYamlText`），序列化时写双引号字符串，兼容 LD51 扁平原格式。

### SampleProject Handle 修正

原 M20 示例 Handle `20402389245123987104` / `21402389245123987105` **超过 `UINT64_MAX`**，`strtoull` 饱和为 `18446744073709551615`。已改为合法值：

| 资产 | Handle |
|------|--------|
| `click.wav` | `10402389245123987104` |
| `SC_Click.hsoundc` | `11402389245123987105` |

---

## 验证

```powershell
cmake --build --preset=debug --target BuildSamplePack HazelTests
.\bin\Debug-windows-x86_64\Tools\BuildSamplePack.exe Hazelnut\SampleProject
cd build\msvc-debug; ctest -C Debug --output-on-failure
```

- `RuntimeTest.AssetManagerRoutesToRuntimeManager` — 加载预置 pack，断言 SoundBank 含 click handle
- `AudioTest.SoundConfigYamlRoundTripLargeHandle` — 大 Handle YAML 往返

---

## 相关模块

- M19 Runtime — [module-19-runtime-launcher.md](./module-19-runtime-launcher.md)
- M20 SoundBank — [module-20-soundbank.md](./module-20-soundbank.md)
