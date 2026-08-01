using System;
using System.Collections.Generic;
using Coral.Managed.Interop;

namespace Hazel
{
	public enum RigidBody2DBodyType
	{
		Static = 0,
		Dynamic,
		Kinematic
	}

	public class TagComponent : Component
	{
		public string Tag
		{
			get
			{
				unsafe
				{
					string? tag = InternalCalls.TagComponent_GetTag(Entity.ID);
					return tag ?? string.Empty;
				}
			}
			set { unsafe { InternalCalls.TagComponent_SetTag(Entity.ID, value); } }
		}
	}

	public class TransformComponent : Component
	{
		public Vector3 Translation
		{
			get
			{
				Vector3 result;
				unsafe { InternalCalls.TransformComponent_GetTranslation(Entity.ID, &result); }
				return result;
			}
			set { unsafe { InternalCalls.TransformComponent_SetTranslation(Entity.ID, &value); } }
		}

		public Vector3 Rotation
		{
			get
			{
				Vector3 result;
				unsafe { InternalCalls.TransformComponent_GetRotation(Entity.ID, &result); }
				return result;
			}
			set { unsafe { InternalCalls.TransformComponent_SetRotation(Entity.ID, &value); } }
		}

		public Vector3 Scale
		{
			get
			{
				Vector3 result;
				unsafe { InternalCalls.TransformComponent_GetScale(Entity.ID, &result); }
				return result;
			}
			set { unsafe { InternalCalls.TransformComponent_SetScale(Entity.ID, &value); } }
		}
	}

	public class RigidBody2DComponent : Component
	{
		public RigidBody2DBodyType BodyType
		{
			get { unsafe { return (RigidBody2DBodyType)InternalCalls.RigidBody2DComponent_GetBodyType(Entity.ID); } }
			set { unsafe { InternalCalls.RigidBody2DComponent_SetBodyType(Entity.ID, (int)value); } }
		}

		public Vector2 LinearVelocity
		{
			get
			{
				Vector2 velocity;
				unsafe { InternalCalls.RigidBody2DComponent_GetLinearVelocity(Entity.ID, &velocity); }
				return velocity;
			}
			set { unsafe { InternalCalls.RigidBody2DComponent_SetLinearVelocity(Entity.ID, &value); } }
		}

		public float GravityScale
		{
			get { unsafe { return InternalCalls.RigidBody2DComponent_GetGravityScale(Entity.ID); } }
			set { unsafe { InternalCalls.RigidBody2DComponent_SetGravityScale(Entity.ID, value); } }
		}

		public void ApplyLinearImpulse(Vector2 impulse, Vector2 offset = default, bool wake = true)
		{
			unsafe { InternalCalls.RigidBody2DComponent_ApplyLinearImpulse(Entity.ID, &impulse, &offset, wake); }
		}
	}

	public class AnimationComponent : Component
	{
		public uint StateIndex
		{
			get { unsafe { return InternalCalls.AnimationComponent_GetStateIndex(Entity.ID); } }
			set { unsafe { InternalCalls.AnimationComponent_SetStateIndex(Entity.ID, value); } }
		}

		public float AnimationTime
		{
			get { unsafe { return InternalCalls.AnimationComponent_GetAnimationTime(Entity.ID); } }
			set { unsafe { InternalCalls.AnimationComponent_SetAnimationTime(Entity.ID, value); } }
		}

		public bool IsAnimationPlaying
		{
			get { unsafe { return InternalCalls.AnimationComponent_GetIsAnimationPlaying(Entity.ID); } }
			set { unsafe { InternalCalls.AnimationComponent_SetIsAnimationPlaying(Entity.ID, value); } }
		}
	}

	public class AudioComponent : Component
	{
		public bool PlayOnAwake
		{
			get { unsafe { return InternalCalls.AudioComponent_GetPlayOnAwake(Entity.ID); } }
			set { unsafe { InternalCalls.AudioComponent_SetPlayOnAwake(Entity.ID, value); } }
		}

		public float Volume
		{
			get { unsafe { return InternalCalls.AudioComponent_GetVolume(Entity.ID); } }
			set { unsafe { InternalCalls.AudioComponent_SetVolume(Entity.ID, value); } }
		}
	}

	public class ScriptComponent : Component
	{
		public NativeInstance<object> Instance
		{
			get { unsafe { return InternalCalls.ScriptComponent_GetInstance(Entity.ID); } }
		}
	}

	public class CameraComponent : Component
	{
		public float VerticalFOV
		{
			get { unsafe { return InternalCalls.CameraComponent_GetVerticalFOV(Entity.ID); } }
			set { unsafe { InternalCalls.CameraComponent_SetVerticalFOV(Entity.ID, value); } }
		}
	}

	public class TextComponent : Component
	{
		public string Text
		{
			get
			{
				unsafe
				{
					string? text = InternalCalls.TextComponent_GetText(Entity.ID);
					return text ?? string.Empty;
				}
			}
			set { unsafe { InternalCalls.TextComponent_SetText(Entity.ID, value); } }
		}

		public Vector4 Color
		{
			get
			{
				Vector4 result;
				unsafe { InternalCalls.TextComponent_GetColor(Entity.ID, &result); }
				return result;
			}
			set { unsafe { InternalCalls.TextComponent_SetColor(Entity.ID, &value); } }
		}

		public float OffsetY
		{
			get { unsafe { return InternalCalls.TextComponent_GetOffsetY(Entity.ID); } }
			set { unsafe { InternalCalls.TextComponent_SetOffsetY(Entity.ID, value); } }
		}
	}

	public class SkinnedMeshComponent : Component { }

	public abstract class Component
	{
		public Entity Entity { get; internal set; } = null!;
	}
}
