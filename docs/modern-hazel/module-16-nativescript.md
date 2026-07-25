# M16 — NativeScript 增强

> 在 M15 C# / Coral 完成前，用 **C++ NativeScript** 验证玩法与编辑器工作流。

## 目标

1. **按名称注册**脚本类（`NativeScriptFactory`）
2. `NativeScriptComponent.ClassName` 场景序列化
3. Play 时自动 `Bind` 并实例化
4. 内置示例：`FlyCameraController`、`JumpController`（Space 给 2D 刚体施加冲量）

---

## 核心 API

| 类型 | 职责 |
|------|------|
| `NativeScriptFactory` | `Register<T>(name)`、`BindSceneScripts(scene)` |
| `RegisterBuiltInNativeScripts()` | 注册引擎自带脚本 |
| `ScriptableEntity` | `GetComponent` / `HasComponent` / `GetEntity` |

Play 流程（`Scene::OnRuntimeStart`）：

1. `RegisterBuiltInNativeScripts()`
2. `NativeScriptFactory::BindSceneScripts`
3. `Physics2DScene::Init`

---

## 场景 YAML

```yaml
NativeScriptComponent:
  ClassName: JumpController
```

---

## Hazelnut Inspector

Add Component → **Native Script** → **Class** 下拉选择已注册类名。

---

## 验证

```powershell
ctest --preset=debug -R NativeScript
```

- 工厂绑定 `FlyCameraController`
- `ClassName` YAML 往返

## 扩展自定义脚本

1. 继承 `ScriptableEntity`，实现 `OnUpdate`
2. 在 `NativeScriptRegistry.cpp` 中 `NativeScriptFactory::Register<MyScript>("MyScript")`
3. 实体上添加 Native Script 组件并选择类名
