# M23 — LD51 Dichotomy Shell

> 阶段 G：PressurePlate / Bridge 机关与 Mechanisms 关卡。

---

## Phase A–F

见 git 历史：`ebca7e1` → `66c6c62`。

---

## Phase G（本提交）

| 项 | 说明 |
|----|------|
| `PressurePlateItem` | 踩踏切换桥梁状态（沙盒区镜像 plate 同样生效） |
| `BridgeUpDown` / `BridgeLeftRight` | 关闭时不可通过（Replicator 踩入即死亡） |
| `IsBlockedTile` | 关闭桥梁 / 未升起地块判定 |
| `Mechanisms.png` | 默认关卡：plate → bridge → goal 线性谜题 |
| HUD 第 4 行 | 显示 `Bridges: OPEN/CLOSED` |

### 操作

| 键 | 功能 |
|----|------|
| W/A/S/D | 沙盒区移动；**先踩蓝色 PressurePlate** 打开桥梁 |
| 等待 10s | Replicator 回放；未开桥时踩桥会死亡 |
| F（按住） | 慢动作 |

### 验证

```powershell
cmake --build --preset=debug --target Hazelnut
dotnet build Hazelnut/LD51Project/assets/Scripts/LD51.csproj -c Debug
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe Hazelnut\LD51Project\LD51.hzproj
```

Play：Player 阶段沿 +Z 走到 plate 并继续；10s 后 Replicator 需桥梁已打开才能到达 Goal。

---

## Phase H（后续）

- TextComponent 世界空间 UI
- TrapType（Axe）与完整 LD51 资产
- Bridge 方向性阻挡（仅垂直/水平格）

---

## 相关模块

- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
