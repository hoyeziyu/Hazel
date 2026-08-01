using System;

using Hazel;

namespace LD51
{
	public class PlayerBase : Entity
	{
		public static readonly float MyEpsilon = 0.1f;

		public float MovementTime = 4.0f;

		protected Vector3Int m_TargetLocation;
		protected Vector3 m_SpawnLocation;

		protected TimeManager? m_TimeManager;
		protected LevelManager? m_LevelManager;

		public bool IsActivePlayer { get; set; }

		protected bool m_IsDead;
		private float m_DeathTimer = 1.0f;

		private AnimationComponent? m_AnimationController;

		protected override void OnCreate()
		{
			m_SpawnLocation = Translation;
			m_TargetLocation = new Vector3Int(Translation);
			if (HasComponent<AnimationComponent>())
				m_AnimationController = GetComponent<AnimationComponent>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			if (m_TimeManager == null)
				m_TimeManager = FindEntityByTag("TimeManager")?.As<TimeManager>();

			if (m_LevelManager == null)
				m_LevelManager = LevelManager.Instance;

			if (m_IsDead)
			{
				m_DeathTimer -= deltaTime;
				if (m_DeathTimer <= 0.0f)
				{
					m_TimeManager?.Reset();
					m_DeathTimer = 1.0f;
					m_IsDead = false;
				}
			}

			Rotation = Vector3.Zero;
			float previousY = Translation.Y;
			m_TargetLocation.Y = (int)Translation.Y;

			Move(deltaTime);

			if (Vector3.Distance(Translation, m_TargetLocation.ToVec3()) < MyEpsilon)
			{
				Translation = new Vector3(m_TargetLocation.X, previousY, m_TargetLocation.Z);
				StopAnimation(0);

				if (m_LevelManager != null && (m_LevelManager.IsDeadlyTile(Translation) || m_LevelManager.IsBlockedTile(Translation)))
				{
					m_LevelManager.OnPlayerDied(this);
					return;
				}

				m_LevelManager?.OnEntitySteppedTile(Translation);

				OnTargetReached();
			}
			else
			{
				Translation = Vector3.Lerp(Translation, m_TargetLocation.ToVec3(), MovementTime * deltaTime);
				Translation = new Vector3(Translation.X, previousY, Translation.Z);
			}
		}

		public void SetSpawnPosition(Vector3 spawnPosition) => m_SpawnLocation = spawnPosition;

		protected void PlayAnimation(uint animationState)
		{
			if (m_AnimationController == null)
				return;

			m_AnimationController.StateIndex = animationState;
			m_AnimationController.IsAnimationPlaying = true;
			m_AnimationController.AnimationTime = 0.0f;
		}

		public void StopAnimation(uint animationState)
		{
			if (m_AnimationController == null)
				return;

			m_AnimationController.StateIndex = animationState;
			m_AnimationController.AnimationTime = 1.0f;
			m_AnimationController.IsAnimationPlaying = false;
		}

		public void SetTargetPosition(Vector3Int target) => m_TargetLocation = target;
		public void SetTargetPosition(Vector3 target) => m_TargetLocation = new Vector3Int(target);

		public void Die()
		{
			if (m_IsDead)
				return;

			m_IsDead = true;
			OnDeath();
			Translation = m_SpawnLocation;
			m_TargetLocation = new Vector3Int(Translation);
			StopAnimation(0);
		}

		protected void PlayMoveAudio()
		{
			if (LevelManager.Instance?.AC == null)
				return;

			var ac = LevelManager.Instance.AC;
			Audio.PostEvent(new AudioCommandID("Move"), ref ac);
		}

		protected virtual void Move(float ts) { }
		protected virtual void OnDeath() { }
		protected virtual void OnTargetReached() { }
	}
}
