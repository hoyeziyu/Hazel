# M23 — LD51 Dichotomy Shell

> 阶段 J：TextComponent 脚本绑定、BridgeUpDown 双轴谜题关卡、Item 标签与配色。

---

## Phase A–I

见 git 历史：`ebca7e1` → `c38519a`。

---

## Phase J（本提交）

| 项 | 说明 |
|----|------|
| `TextComponent` C# 绑定 | `Text` / `Color` / `OffsetY`；新增 `Vector4` |
| 脚本演示 | 桥梁开关时通过 `TextComponent.Color` 切换绿/金标签 |
| `BridgeUpDown.png` | 三列 plate + 左/中/右 混合 LR/UD 桥，Goal 居中 |
| Item 标签 | `BR-LR` / `BR-UD` 区分桥梁类型 |

### 操作

| 键 | 功能 |
|----|------|
| W/A/S/D | 沙盒移动 |
| 左列 PLATE (x=-2) | 切换左桥 (BridgeLeftRight) |
| 中列 PLATE (x=0) | 切换中桥 (BridgeUpDown) |
| 右列 PLATE (x=+2) | 切换右桥 (BridgeLeftRight) |
| 等待 10s | Replicator 须走已开桥路径至 Goal |

桥梁关闭时：LR 阻挡左右移动，UD 阻挡前后移动。HUD 第 4 行 `Bridges: open/total`。

### C# 示例

```csharp
var text = entity.GetComponent<TextComponent>();
if (text != null)
{
    text.Text = "GOAL";
    text.Color = new Vector4(1, 0.9f, 0.2f, 1);
    text.OffsetY = 0.8f;
}
```

### 验证

```powershell
cmake --build --preset=debug --target hazel-engine HazelTests Hazelnut
dotnet build Hazelnut/LD51Project/assets/Scripts/LD51.csproj -c Debug
cd build\msvc-debug; ctest -C Debug --output-on-failure
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe Hazelnut\LD51Project\LD51.hzproj
```

---

## Phase K（后续）

- 导入 HazelGame 真实 glTF 模型替换占位立方体
- TextComponent 编辑器实时预览
- 更多 LD51 字母关卡

---

## 相关模块

- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
