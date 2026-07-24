# Modern Hazel 学习笔记

对照 **[StudioCherno Hazel 2024.1](E:/project/cherno/HazelEngine)**，在本地 **CMake/OpenGL 教程引擎** 上逐步仿制 workflow 与架构概念。

## 仓库对照

| 路径 | 用途 |
|------|------|
| `E:\project\cherno\Hazel` | 你的主线（实现缩小版功能） |
| `E:\project\cherno\HazelEngine` | StudioCherno 只读参考 |

## 模块进度

| 模块 | 主题 | 状态 | 笔记 |
|------|------|------|------|
| M1 | 项目系统 | ✅ 完成 | [module-01-project.md](./module-01-project.md) |
| M2 | Play 模式 | ✅ 完成 | [module-02-play.md](./module-02-play.md) |
| M3 | Content Browser | ✅ 完成 | [module-03-content-browser.md](./module-03-content-browser.md) |
| M4 | Editor Camera | ✅ 完成 | [module-04-editor-camera.md](./module-04-editor-camera.md) |
| M5 | 资产系统 | ✅ 完成 | [module-05-asset-system.md](./module-05-asset-system.md) |
| M6 | Prefab / AssetPack | ✅ 完成 | [module-06-prefab-assetpack.md](./module-06-prefab-assetpack.md) |
| M7 | SceneRenderer（3D 视口） | ✅ 完成 | [module-07-scene-renderer.md](./module-07-scene-renderer.md) |
| M8 | StaticMesh 资产 | ✅ 完成 | [module-08-staticmesh.md](./module-08-staticmesh.md) |
| M9 | Material 资产（Albedo 纹理） | ✅ 完成 | [module-09-material.md](./module-09-material.md) |

完整路线见 [ROADMAP.md](./ROADMAP.md)。

## 学习流程（每个模块）

1. 读 `HazelEngine` 对应源码  
2. 在本仓库实现缩小版  
3. 更新本目录笔记  
4. 运行 `Hazelnut.exe` 验收  
