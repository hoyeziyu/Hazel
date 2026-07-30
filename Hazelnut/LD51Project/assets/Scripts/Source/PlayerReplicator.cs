using System.Collections.Generic;

using Hazel;

namespace LD51
{
	public struct PlayerMovement
	{
		public Vector3 Position;
		public float TimeSpentOnTile;
	}

	public class PlayerReplicator : PlayerBase
	{
		private readonly LinkedList<PlayerMovement> m_RecordedMoves = new LinkedList<PlayerMovement>();
		private float m_TileTimer;
		private bool m_ReplicateEnabled;

		protected override void OnCreate()
		{
			base.OnCreate();
			IsActivePlayer = false;
		}

		protected override void OnUpdate(float deltaTime)
		{
			if (!m_ReplicateEnabled && m_RecordedMoves.Count > 0)
			{
				var movement = m_RecordedMoves.Last!.Value;
				m_RecordedMoves.RemoveLast();
				movement.TimeSpentOnTile += deltaTime;
				m_RecordedMoves.AddLast(movement);
			}

			if (!m_ReplicateEnabled)
				return;

			LevelManager.Instance?.ShowNeighbouringTiles(m_TargetLocation);
			base.OnUpdate(deltaTime);
		}

		public void SetReplicateEnabled(bool enabled)
		{
			IsActivePlayer = enabled;
			m_ReplicateEnabled = enabled;
			m_IsDead = false;

			if (enabled)
				LevelManager.Instance?.ShowNeighbouringTiles(new Vector3Int(Translation));
		}

		protected override void Move(float ts)
		{
			if (!m_ReplicateEnabled)
				return;

			if (m_TileTimer <= 0.0f && m_RecordedMoves.Count > 0)
			{
				PlayerMovement movement = PopMove();
				Vector3 targetLocation = Translation + (movement.Position * -1.0f);
				m_TargetLocation = new Vector3Int(targetLocation);
				m_TileTimer = movement.TimeSpentOnTile;
				PlayAnimation(0);
				PlayMoveAudio();
			}
			else
			{
				m_TileTimer -= ts;
			}
		}

		protected override void OnTargetReached()
		{
			if (m_LevelManager != null && m_LevelManager.IsGoalTile(Translation))
			{
				m_RecordedMoves.Clear();
				m_TileTimer = 0.0f;
				m_ReplicateEnabled = false;
				SetTargetPosition(Translation);
				Log.Info("PlayerReplicator: reached goal!");

				if (LevelManager.Instance?.AC != null)
				{
					var ac = LevelManager.Instance.AC;
					Audio.PostEvent(new AudioCommandID("Click"), ref ac);
				}
			}
		}

		protected override void OnDeath()
		{
			m_RecordedMoves.Clear();
			m_TileTimer = 0.0f;
			m_ReplicateEnabled = false;
		}

		private PlayerMovement PopMove()
		{
			if (m_RecordedMoves.Count == 0)
				return new PlayerMovement();

			var movement = m_RecordedMoves.First!.Value;
			m_RecordedMoves.RemoveFirst();
			return movement;
		}

		public void RecordMove(Vector3 move)
		{
			m_RecordedMoves.AddLast(new PlayerMovement
			{
				Position = move,
				TimeSpentOnTile = 0.0f
			});
		}
	}
}
