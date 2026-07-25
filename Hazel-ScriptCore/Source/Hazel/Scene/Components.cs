namespace Hazel
{
	public enum RigidBody2DBodyType
	{
		Static = 0,
		Dynamic,
		Kinematic
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

	public abstract class Component
	{
		public Entity Entity { get; internal set; } = null!;
	}
}
