# M22 — Editor Play-from-Pack + Runtime 硬化

> 承接 M21 Pack/SoundBank 闭环，让 **Editor Play 与 Hazel-Runtime 走同一条 AssetPack 路径**，并在需要时强制 SoundBank。

---

## 目标

| 项 | 说明 |
|----|------|
| Editor Play | Play 前自动 Build Pack，从 `AssetPack.hap` 加载场景（替代 `CopyTo`） |
| Stop 恢复 | Stop 时 `SetActive` 恢复 Editor 资产管线 |
| Runtime 校验 | Pack 含 `SoundConfig` 时，`SoundBank.hsb` 缺失或加载失败 → 启动失败 |
| Launcher 部署 | POST_BUILD 复制 `Sample.dll` 到 bundled SampleProject（与 Runtime 一致） |

---

## Editor Play 流程

```
Alt+P (Play)
  1. 保存当前 .hazel 到磁盘
  2. AssetPack::CreateFromActiveProject → .hap + .hdat + SoundBank
  3. AssetPack::Load + Project::SetActiveRuntime
  4. RequiresSoundBank ? 必须 LoadSoundBank 成功
  5. RuntimeAssetManager::LoadScene(activeSceneHandle)
  6. OnRuntimeStart()

Alt+P (Stop)
  1. OnRuntimeStop
  2. Project::SetActive(project) — 恢复 EditorAssetManager
  3. ReloadScriptEngine
```

`ResolveActiveSceneHandle()`：优先 `m_ActiveScenePath` 相对 assets 的 registry Handle，否则回退 `StartScene`。

---

## AssetPack::RequiresSoundBank

扫描 Pack 索引中所有场景的 `AssetInfo.Type == SoundConfig`。SampleProject Main 场景引用 `SC_Click`，因此 Runtime 与 Play 均要求 `SoundBank.hsb` 存在。

---

## 与 M19「刻意未做」的关系

| M19 项 | M22 状态 |
|--------|----------|
| Play 从 Pack 加载 | ✅ 已实现 |
| Pack 内嵌 C# DLL | 仍从磁盘 `Scripts/Binaries/` |
| Tiering / ShaderPack | 未做 |

---

## 验证

```powershell
cmake --build --preset=debug --target Hazelnut Hazel-Runtime HazelTests
cd build\msvc-debug; ctest -C Debug --output-on-failure
```

- Hazelnut 打开 SampleProject → Play：应听到 click 音，与 Runtime 行为一致
- 删除 `SoundBank.hsb` 后 Play / Runtime 应报错退出

---

## 相关模块

- M19 Runtime — [module-19-runtime-launcher.md](./module-19-runtime-launcher.md)
- M21 Pack 闭环 — [module-21-runtime-pack-closure.md](./module-21-runtime-pack-closure.md)
