# M23 — LD51 Dichotomy Shell

> 阶段 I：TextComponent、按列 plate/bridge 联动、DualMechanisms 关卡。

---

## Phase A–H

见 git 历史：`ebca7e1` → `0778ed2`。

---

## Phase I（本提交）

| 项 | 说明 |
|----|------|
| `TextComponent` | 序列化 + Hierarchy 编辑器 + Play 时投影为世界标签（slot 8+） |
| Item Prefab 标签 | GOAL / PLATE / AXE / BRIDGE 通过 TextComponent 定义 |
| 按列联动 | 踩 PressurePlate 仅切换**同 X 列**桥梁 |
| `DualMechanisms.png` | 左右两列 plate+bridge，需分别激活 |

### 操作

| 键 | 功能 |
|----|------|
| W/A/S/D | 沙盒移动 |
| 左列 PLATE (x=-2) | 打开左桥 |
| 右列 PLATE (x=+2) | 打开右桥 |
| 等待 10s | Replicator 须走已开桥路径至 Goal |

HUD 第 4 行显示 `Bridges: open/total`。

### 验证

```powershell
cmake --build --preset=debug --target hazel-engine HazelTests Hazelnut
dotnet build Hazelnut/LD51Project/assets/Scripts/LD51.csproj -c Debug
cd build\msvc-debug; ctest -C Debug --output-on-failure
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe Hazelnut\LD51Project\LD51.hzproj
```

---

## Phase J（后续）

- TextComponent 脚本绑定
- 完整 LD51 美术资产
- BridgeUpDown 双轴谜题关卡

---

## 相关模块

- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
