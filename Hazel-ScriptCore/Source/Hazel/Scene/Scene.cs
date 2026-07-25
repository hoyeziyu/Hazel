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
	}
}
