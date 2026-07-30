# M23 — LD51 Dichotomy Shell

> 阶段 F：Runtime HUD、致命地块、慢动作/FOV、第二关卡。

---

## Phase A–E

见 git 历史：`ebca7e1` → `6df5c94`。

---

## Phase F（本提交）

| 项 | 说明 |
|----|------|
| `RuntimeHUD` + `HUD` C# | 脚本 `HUD.SetLine` / `Clear`，Play 模式 Viewport ImGui 叠加 |
| `Time.TimeScale` | 全局时间缩放；`TimeManager` 慢动作 + 相机 FOV |
| `CameraComponent.VerticalFOV` | 脚本可读写透视 FOV（度） |
| `SpikesTile` / `LavaTile` | 新 Prefab + `IsDeadlyTile` + 玩家死亡 |
| `Challenge.png` | 第二关卡（Spikes 行 + Lava 格 + 中心 Goal） |

### 操作

| 键 | 功能 |
|----|------|
| W/A/S/D 或方向键 | 沙盒区移动（录制路径） |
| Space | 预览相邻沙盒格（日志） |
| F（按住） | 慢动作 + 缩小 FOV |
| 等待 10s | 自动切换至 Replicator 回放 |
| 踩 Spikes/Lava | 死亡并重置回合计时 |

Viewport 左上角 HUD 显示倒计时、当前控制角色（Player / Replicator）、慢动作提示。

### 验证

```powershell
cmake --build --preset=debug --target hazel-engine HazelTests Hazelnut
dotnet build Hazelnut/LD51Project/assets/Scripts/LD51.csproj -c Debug
cd build\msvc-debug; ctest -C Debug --output-on-failure
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe Hazelnut\LD51Project\LD51.hzproj
```

Play 后：HUD 显示倒计时；Challenge 关卡含致命地块；Replicator 回放时可踩 lava/spikes 触发死亡。

---

## Phase G（后续）

- Bridge / PressurePlate 机关
- 完整 LD51 资产导入
- TextComponent 世界空间 UI

---

## 相关模块

- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
