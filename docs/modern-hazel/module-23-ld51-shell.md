# M23 — LD51 Dichotomy Shell

> 阶段 D：LD51 仿制 **层级 + Prefab + C# 脚本 + PNG 关卡 + 双角色循环**。

---

## Phase A–D

见 git 历史：`ebca7e1` → `2901a27` → `17bc280`。

---

## Phase E（本提交）

| 项 | 说明 |
|----|------|
| `PlayerBase` / `Player` | WASD 沙盒移动，录制步进 |
| `PlayerReplicator` | 回放录制路径，Goal 检测 |
| `TimeManager` | 10 秒倒计时（日志），触发 `SwitchRound` |
| `LevelManager` 扩展 | 地块升起动画、双区切换、`HasValidSandboxTile` |
| 场景 | Player（蓝）+ Replicator（红）+ TimeManager + Audio |

### 操作

| 键 | 功能 |
|----|------|
| W/A/S/D 或方向键 | 沙盒区移动（录制路径） |
| Space | 预览相邻沙盒格（日志） |
| 等待 10s | 自动切换至 Replicator 回放 |

### 验证

```powershell
cmake --build --preset=debug --target Hazelnut
dotnet build Hazelnut/LD51Project/assets/Scripts/LD51.csproj -c Debug
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe Hazelnut\LD51Project\LD51.hzproj
```

Play 后：Level 区地块从 Y=-100 升起；WASD 移动蓝色 Player；10 秒后红色 Replicator 回放路径。

---

## Phase F（后续）

- TextComponent 倒计时 UI
- 更多 Tile Prefab（Lava、Spikes、Bridge）
- Player 死亡 / 慢动作 / FOV
- 完整 LD51 资产导入

---

## 相关模块

- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
