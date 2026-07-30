using System;

namespace Hazel
{
	public static class Scene
	{
		public static Entity CreateEntity(string tag = "Entity")
		{
			unsafe { return new Entity(InternalCalls.Scene_CreateEntity(tag)); }
		}

		public static void DestroyEntity(Entity entity)
		{
			if (entity == null)
				return;

			unsafe
			{
				if (!InternalCalls.Scene_IsEntityValid(entity.ID))
					return;
				InternalCalls.Scene_DestroyEntity(entity.ID);
			}
		}

		public static Entity? FindEntityByTag(string tag)
		{
			unsafe
			{
				ulong entityID = InternalCalls.Scene_FindEntityByTag(tag);
				return entityID != 0 ? new Entity(entityID) : null;
			}
		}

		public static Entity[] GetEntities()
		{
			unsafe
			{
				using var entityIDs = InternalCalls.Scene_GetEntities();
				var entities = new Entity[entityIDs.Length];
				for (int i = 0; i < entityIDs.Length; i++)
					entities[i] = new Entity(entityIDs[i]);
				return entities;
			}
		}

		public static Entity? InstantiatePrefab(Prefab prefab)
		{
			unsafe
			{
				ulong entityID = InternalCalls.Scene_InstantiatePrefab(prefab.Handle.m_Handle);
				return entityID == 0 ? null : new Entity(entityID);
			}
		}

		public static Entity? InstantiatePrefab(Prefab prefab, Vector3 translation)
		{
			unsafe
			{
				ulong entityID = InternalCalls.Scene_InstantiatePrefabWithTranslation(prefab.Handle.m_Handle, &translation);
				return entityID == 0 ? null : new Entity(entityID);
			}
		}

		public static Entity? InstantiatePrefab(Prefab prefab, Transform transform)
		{
			unsafe
			{
				ulong entityID = InternalCalls.Scene_InstantiatePrefabWithTransform(
					prefab.Handle.m_Handle, &transform.Position, &transform.Rotation, &transform.Scale);
				return entityID == 0 ? null : new Entity(entityID);
			}
		}

		public static Entity? InstantiatePrefabWithParent(Prefab prefab, Entity parent)
		{
			unsafe
			{
				ulong entityID = InternalCalls.Scene_InstantiatePrefab(prefab.Handle.m_Handle);
				return entityID == 0 ? null : new Entity(entityID) { Parent = parent };
			}
		}

		public static Entity? InstantiatePrefabWithParent(Prefab prefab, Vector3 translation, Entity parent)
		{
			unsafe
			{
				ulong entityID = InternalCalls.Scene_InstantiateChildPrefabWithTranslation(parent.ID, prefab.Handle.m_Handle, &translation);
				return entityID == 0 ? null : new Entity(entityID);
			}
		}

		public static Entity? InstantiatePrefabWithParent(Prefab prefab, Transform transform, Entity parent)
		{
			unsafe
			{
				ulong entityID = InternalCalls.Scene_InstantiateChildPrefabWithTransform(
					parent.ID, prefab.Handle.m_Handle, &transform.Position, &transform.Rotation, &transform.Scale);
				return entityID == 0 ? null : new Entity(entityID);
			}
		}
	}
}
