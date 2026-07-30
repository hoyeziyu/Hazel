using System;
using System.Collections.Generic;
using Coral.Managed.Interop;

namespace Hazel
{
	[EditorAssignable]
	public class Entity
	{
		private Entity? m_Parent;
		private Dictionary<Type, Component> m_ComponentCache = new Dictionary<Type, Component>();

		protected Entity() { ID = 0; }

		internal Entity(ulong id)
		{
			ID = id;
		}

		public readonly ulong ID;

		public string Tag
		{
			get
			{
				unsafe
				{
					string? tag = InternalCalls.TagComponent_GetTag(ID);
					return tag ?? string.Empty;
				}
			}
			set { unsafe { InternalCalls.TagComponent_SetTag(ID, value); } }
		}

		public Vector3 Translation
		{
			get
			{
				Vector3 result;
				unsafe { InternalCalls.TransformComponent_GetTranslation(ID, &result); }
				return result;
			}
			set
			{
				unsafe { InternalCalls.TransformComponent_SetTranslation(ID, &value); }
			}
		}

		public Vector3 Rotation
		{
			get
			{
				Vector3 result;
				unsafe { InternalCalls.TransformComponent_GetRotation(ID, &result); }
				return result;
			}
			set
			{
				unsafe { InternalCalls.TransformComponent_SetRotation(ID, &value); }
			}
		}

		public Vector3 Scale
		{
			get
			{
				Vector3 result;
				unsafe { InternalCalls.TransformComponent_GetScale(ID, &result); }
				return result;
			}
			set
			{
				unsafe { InternalCalls.TransformComponent_SetScale(ID, &value); }
			}
		}

		public Entity? Parent
		{
			get
			{
				unsafe
				{
					ulong parentID = InternalCalls.Entity_GetParent(ID);
					if (parentID == 0)
						return null;

					if (m_Parent == null || m_Parent.ID != parentID)
						m_Parent = InternalCalls.Scene_IsEntityValid(parentID) ? new Entity(parentID) : null;
				}

				return m_Parent;
			}

			set
			{
				unsafe { InternalCalls.Entity_SetParent(ID, value != null ? value.ID : 0); }
			}
		}

		public Entity[] Children
		{
			get
			{
				unsafe
				{
					using var childIDs = InternalCalls.Entity_GetChildren(ID);
					var children = new Entity[childIDs.Length];
					for (int i = 0; i < childIDs.Length; i++)
						children[i] = new Entity(childIDs[i]);
					return children;
				}
			}
		}

		public bool HasComponent<T>() where T : Component
		{
			unsafe { return InternalCalls.Entity_HasComponent(ID, typeof(T)); }
		}

		public bool HasComponent(Type type)
		{
			unsafe { return InternalCalls.Entity_HasComponent(ID, type); }
		}

		public T? GetComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);

			if (!HasComponent<T>())
			{
				m_ComponentCache.Remove(componentType);
				return null;
			}

			if (!m_ComponentCache.TryGetValue(componentType, out Component? cached))
			{
				var component = new T { Entity = this };
				m_ComponentCache.Add(componentType, component);
				return component;
			}

			return cached as T;
		}

		public Entity? Instantiate(Prefab prefab) => Scene.InstantiatePrefab(prefab);
		public Entity? Instantiate(Prefab prefab, Vector3 translation) => Scene.InstantiatePrefab(prefab, translation);
		public Entity? Instantiate(Prefab prefab, Transform transform) => Scene.InstantiatePrefab(prefab, transform);

		public Entity? InstantiateChild(Prefab prefab) => Scene.InstantiatePrefabWithParent(prefab, this);
		public Entity? InstantiateChild(Prefab prefab, Vector3 translation) => Scene.InstantiatePrefabWithParent(prefab, translation, this);
		public Entity? InstantiateChild(Prefab prefab, Transform transform) => Scene.InstantiatePrefabWithParent(prefab, transform, this);

		public bool Is<T>() where T : Entity
		{
			var sc = GetComponent<ScriptComponent>();
			if (sc == null)
				return false;

			return sc.Instance.Get() is T;
		}

		public T? As<T>() where T : Entity
		{
			return GetComponent<ScriptComponent>()?.Instance.Get() as T;
		}

		public void Destroy() => Scene.DestroyEntity(this);
		public void Destroy(Entity other) => Scene.DestroyEntity(other);

		protected virtual void OnCreate() { }
		protected virtual void OnUpdate(float ts) { }
		protected virtual void OnDestroy() { }
	}
}
