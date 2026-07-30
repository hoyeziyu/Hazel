# M23 — LD51 Dichotomy Shell

> 阶段 D 入口：LD51 仿制 **Parent/Children + Prefab 树 + C# 脚本 + PNG 关卡 + LevelManager**。

---

## Phase A（`ebca7e1`）— 实体层级 + Prefab 实例化

## Phase B（`0b15da0`）— C# ScriptCore + AudioCommandRegistry

## Phase C（`2901a27`）— Prefab 脚本字段 + LD51Project 壳

---

## Phase D（本提交）

| 项 | 说明 |
|----|------|
| 最小 Prefab 集 | `GrassTile` / `EmptyTile` / `GoalItem` / `SandboxTile` / `SectionContainer` / `LevelContainer` |
| `LevelManager.cs` | 读 `Starter.png` → 实例化 Sandbox + Level 双区地块 |
| `LevelSection.cs` | 地块字典与 Goal 坐标 |
| 场景 | `Main.hazel` — Camera + Sun + LevelManager（Prefab 脚本字段） |
| 构建 | `HazelDotNetBuild` 含 LD51.dll；Hazelnut POST_BUILD 部署 |

### 打开并 Play

```powershell
cmake --build --preset=debug --target Hazelnut
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe Hazelnut\LD51Project\LD51.hzproj
```

Play 后 `LevelManager.OnCreate` 会从 `assets/Levels/Starter.png` 生成 11×11 地块（Sandbox 区 + Level 区）。

---

## Phase E（后续）

- Player / PlayerReplicator / TimeManager 脚本
- 更多 Tile Prefab（Lava、Spikes 等）与 LD51 完整资产导入
- TextComponent 或 ImGui 倒计时

---

## 验证

```powershell
cmake --build --preset=debug --target HazelTests Hazelnut
cd build\msvc-debug; ctest -C Debug --output-on-failure
dotnet build Hazelnut/LD51Project/assets/Scripts/LD51.csproj -c Debug
```

---

## 相关模块

- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
