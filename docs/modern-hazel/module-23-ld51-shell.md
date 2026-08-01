# M23 — LD51 Dichotomy Shell

> 阶段 K：TextComponent 编辑器预览、LD51 简易网格、字母关卡画廊。

---

## Phase A–J

见 git 历史：`ebca7e1` → `37bb2c7`。

---

## Phase K（本提交）

| 项 | 说明 |
|----|------|
| TextComponent 编辑器预览 | Edit 模式视口投影世界标签（与 Play 一致） |
| 简易网格 | `TileFlat`（平面地块）、`GoalGem`（八面体）；OBJ 源文件供 Content Browser 导入 |
| 字母关卡 | `D.png` / `S.png` / `E.png` |
| LevelGallery | 按 **N** 循环切换关卡；HUD 第 6 行显示关卡名 |

### 关卡说明

| 关卡 | 内容 |
|------|------|
| **D** | 单列 Plate + BridgeUpDown + Goal（入门） |
| **S** | 十字 Spikes + Goal |
| **E** | Plate + Axe + BridgeLeftRight + Goal |
| BridgeUpDown / DualMechanisms / … | Phase I–J 谜题 |

### 操作

| 键 | 功能 |
|----|------|
| W/A/S/D | 沙盒移动 |
| **N** | 下一关卡（画廊循环） |
| F | 慢动作 |
| Space | 揭示相邻地块（调试） |

### 验证

```powershell
cmake --build --preset=debug --target hazel-engine HazelTests Hazelnut
dotnet build Hazelnut/LD51Project/assets/Scripts/LD51.csproj -c Debug
cd build\msvc-debug; ctest -C Debug --output-on-failure
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe Hazelnut\LD51Project\LD51.hzproj
```

Edit 模式打开带 TextComponent 的 Prefab 即可看到标签预览。

---

## Phase L（后续）

- 从 HazelGame 导入 glTF（Bridge、SpikeTile 等）
- TextComponent 编辑器 Gizmo / 拖拽 OffsetY
- 完整 26 字母关卡集

---

## 相关模块

- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
