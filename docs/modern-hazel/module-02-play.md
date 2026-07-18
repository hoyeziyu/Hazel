# M2 — Play 模式

> 对照：`HazelEngine/Hazelnut/src/EditorLayer.cpp` → `OnScenePlay` / `OnSceneStop`  
> 实现：`Hazel/Scene/Scene.*`、`Hazelnut/src/EditorLayer.*`

## StudioCherno 怎么做

- `SceneState`：`Edit / Play / Pause / Simulate`
- `m_EditorScene`：编辑态源场景
- `m_RuntimeScene`：Play 时 `CopyTo` 出来的副本
- Play：`OnRuntimeStart()` → 更新脚本/物理 → `OnUpdateRuntime`
- Stop：`OnRuntimeStop()` → 丢弃 runtime，回到 editor

## 我们的缩小版（Edit / Play）

### 引擎 `Scene`

| API | 行为 |
|-----|------|
| `CopyTo(target)` | 内存复制 Tag/Transform/Camera/Sprite/NativeScript；脚本 Instance 重置 |
| `OnUpdateEditor` | 仅渲染（Edit 下脚本不跑） |
| `OnUpdateRuntime` | 脚本 + 渲染 |
| `OnRuntimeStart/Stop` | Stop 时销毁 NativeScript 实例 |

### 编辑器 `EditorLayer`

| 功能 | 说明 |
|------|------|
| `m_EditorScene` | 原 `m_ActiveScene`，Hierarchy 始终指向它 |
| `m_RuntimeScene` | Play 期间的副本 |
| Viewport **Play/Stop** 按钮 | Play 时红色 Stop |
| **Alt+P** | 切换 Play/Stop |
| Play 时 | 禁用 Gizmo、禁用编辑相机控制器 |
| New/Open/Close Scene/Project | 前先 `OnSceneStop()` |

## 验收清单

- [ ] Edit 下移动实体 → Play → 从当前布局开始运行
- [ ] Play 中改 runtime（若有脚本）→ Stop → editor 布局未被 Play 污染
- [ ] Play 时 Gizmo 与 Q/W/E/R 无效
- [ ] Project 面板 `State: Edit/Play` 正确
- [ ] Alt+P 与 Play/Stop 按钮行为一致

## 刻意未做

- Pause / Simulate / 物理
- Play 时 Hierarchy 切到 runtime
- Play/Stop 图标按钮

## 下一模块（M3）

Content Browser：对照 `ContentBrowserPanel`，浏览项目 `assets/` 并双击打开 `.hazel` 场景。
