# Modern Hazel 仿制路线图

对照 **[HazelEngine 2024.1](E:/project/cherno/HazelEngine)**（Vulkan + C# + Jolt + 音频 + 动画），在本地 **CMake/OpenGL** 引擎上分模块缩小实现。

游戏参考目录：**[HazelGame](E:/project/cherno/HazelGame)**（Fragile、LD50 Forest、LD51 Dichotomy）。

---

## 阶段 A — 编辑器与资产（进行中）

| 模块 | 主题 | 状态 | 笔记 |
|------|------|------|------|
| M1 | 项目系统 | ✅ | [module-01-project.md](./module-01-project.md) |
| M2 | Play 模式 | ✅ | [module-02-play.md](./module-02-play.md) |
| M3 | Content Browser | ✅ | [module-03-content-browser.md](./module-03-content-browser.md) |
| M4 | Editor Camera | ✅ | [module-04-editor-camera.md](./module-04-editor-camera.md) |
| M5 | 资产系统 | ✅ | [module-05-asset-system.md](./module-05-asset-system.md) |
| M6 | Prefab / AssetPack | ✅ | [module-06-prefab-assetpack.md](./module-06-prefab-assetpack.md) |
| M7 | SceneRenderer 3D | ✅ | [module-07-scene-renderer.md](./module-07-scene-renderer.md) |
| M8 | StaticMesh | ✅ | [module-08-staticmesh.md](./module-08-staticmesh.md) |
| M9 | Material（Albedo 纹理） | ✅ | [module-09-material.md](./module-09-material.md) |
| M10 | 方向光 + 简单光照 | ✅ | [module-10-11-lighting-stats.md](./module-10-11-lighting-stats.md) |
| M11 | SceneRenderer Stats / DrawCall | ✅ | [module-10-11-lighting-stats.md](./module-10-11-lighting-stats.md) |
| M12 | Assimp 模型导入 | ✅ | [module-12-assimp-import.md](./module-12-assimp-import.md) |

---

## 阶段 B — 物理与脚本

| 模块 | HazelEngine 对照 | 说明 | 状态 | 笔记 |
|------|------------------|------|------|------|
| M13 | Box2D / Physics2D | Sandbox2D、LD50 Physics2D 场景 | ✅ | [module-13-physics2d.md](./module-13-physics2d.md) |
| M14 | Jolt 3D 物理（可选） | Fragile、Forest 射击/碰撞 |
| M15 | C# Script（Coral） | .NET 9 + ScriptCore + Sample.dll | ✅ | [module-15-csharp-scripting.md](./module-15-csharp-scripting.md) |
| M16 | NativeScript 增强 | 无 C# 时的 C++ 脚本替代 | ✅ | [module-16-nativescript.md](./module-16-nativescript.md) |

---

## 阶段 C — 音频、动画、运行时

| 模块 | HazelEngine 对照 | 说明 | 状态 | 笔记 |
|------|------------------|------|------|------|
| M17 | miniaudio 基础 | LD50/LD51 音效 | ✅ | [module-17-miniaudio.md](./module-17-miniaudio.md) |
| M18 | AnimationGraph 缩小版 | LD51 玩家动画 | ✅ | [module-18-animation.md](./module-18-animation.md) |
| M19 | Hazel-Runtime + Launcher | 打包运行 | ✅ | [module-19-runtime-launcher.md](./module-19-runtime-launcher.md) |
| M20 | SoundGraph / SoundBank | LD51 SoundBank | ✅ | [module-20-soundbank.md](./module-20-soundbank.md)（Phase A：SoundConfig + SoundBank） |
| M21 | Runtime Pack 闭环 | Pack + SoundBank + 预置资产 | ✅ | [module-21-runtime-pack-closure.md](./module-21-runtime-pack-closure.md) |
| M22 | Editor Play-from-Pack | Play 走 Pack + Runtime 硬化 | ✅ | [module-22-play-from-pack.md](./module-22-play-from-pack.md) |

---

## 阶段 D — 三款游戏仿制

| 游戏 | 目录 | 依赖模块 | 核心玩法 |
|------|------|----------|----------|
| **Fragile** | `HazelGame/Fragile-main` | M14 物理、第三人称、Prefab | 行星重力、导弹 |
| **Forest (LD50)** | `HazelGame/LD50-main` | M15 脚本、M17 音频、M14 物理、程序化生成 | FPS horror、分块森林 |
| **Dichotomy (LD51)** | `HazelGame/LD51-main` | M18 动画、M17 音频、关卡 PNG | 10 秒双区域 |

游戏逻辑主要在 **C# 脚本**；阶段 B 的 M15 完成前，可先用 **NativeScript** 验证玩法原型。

---

## 与 HazelEngine 的架构差异（长期）

| 能力 | HazelEngine | 本仓库 |
|------|-------------|--------|
| 渲染 API | Vulkan 延迟管线 | OpenGL 前向 |
| 构建 | Premake5 | CMake + vcpkg |
| C++ | C++20 | C++17 |
| 脚本 | C# / Coral | C# / Coral（M15 精简版） |

---

## 每个模块的标准流程

1. 阅读 HazelEngine 对应源码  
2. 在本仓库实现 OpenGL 缩小版  
3. 编写 `docs/modern-hazel/module-XX-*.md`  
4. `cmake --build` + `ctest` + `HazelBench`（可选）  
5. 提交分支 + PR  

---

## 权限与依赖说明

如需以下能力，请在本机或 CI 预先准备：

- **Assimp**（M12）：vcpkg 添加 `assimp`  
- **Box2D**（M13）：vcpkg 或 vendor  
- **Jolt Physics**（M14）：vendor 子模块  
- **.NET 8 SDK + Coral**（M15）：脚本系统，需 Windows 开发环境  
- **miniaudio**（M17）：header-only，较易接入  

当前 M9 无额外 vcpkg 依赖。
