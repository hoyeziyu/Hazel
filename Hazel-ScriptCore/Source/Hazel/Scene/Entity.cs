namespace Hazel
{
	[EditorAssignable]
	public class Entity
	{
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

		protected virtual void OnCreate() { }
		protected virtual void OnUpdate(float ts) { }
		protected virtual void OnDestroy() { }
	}
}
