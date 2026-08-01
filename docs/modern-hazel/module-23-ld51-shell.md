# M23 — LD51 Dichotomy Shell

> 阶段 H：世界空间标签、Axe 陷阱、桥梁方向性阻挡。

---

## Phase A–G

见 git 历史：`ebca7e1` → `5c43d24`。

---

## Phase H（本提交）

| 项 | 说明 |
|----|------|
| `HUD.SetWorldLabel` | 世界坐标文字，Play 模式 Viewport 投影渲染 |
| `AxeTrapItem` | 2 秒交替摆动的致命斧陷阱 |
| `IsMoveBlocked` | 桥梁方向性阻挡（LeftRight 挡 X，UpDown 挡 Z） |
| `TrapsAndBridges.png` | 默认关卡：Plate → Axe → Bridge → Goal |

### 操作

| 键 | 功能 |
|----|------|
| W/A/S/D | 沙盒移动；先踩 PLATE 开桥 |
| 观察 Axe | 红色斧条 2 秒周期；**收回时**再通过 |
| 等待 10s | Replicator 回放（时机与路径须一致） |

世界标签：Goal / Plate / Axe 显示在对应地块上方（黄色投影文字）。

### 验证

```powershell
cmake --build --preset=debug --target hazel-engine HazelTests Hazelnut
dotnet build Hazelnut/LD51Project/assets/Scripts/LD51.csproj -c Debug
cd build\msvc-debug; ctest -C Debug --output-on-failure
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe Hazelnut\LD51Project\LD51.hzproj
```

---

## Phase I（后续）

- 完整 TextComponent 组件（序列化 + 编辑器）
- 完整 LD51 资产导入
- 多 plate / 多 bridge 联动

---

## 相关模块

- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
