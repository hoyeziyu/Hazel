using Hazel;

namespace LD51
{
	public class Player : PlayerBase
	{
		public Entity PlayerReplicatorEntity = null!;
		private PlayerReplicator? m_Replicator;
		private bool m_InputEnabled = true;

		protected override void OnCreate()
		{
			base.OnCreate();
			IsActivePlayer = true;
		}

		protected override void OnUpdate(float deltaTime)
		{
			if (Input.IsKeyPressed(KeyCode.F))
				m_TimeManager?.EnterSlowMotion();

			if (!IsActivePlayer)
				return;

			if (m_Replicator == null)
				m_Replicator = PlayerReplicatorEntity.As<PlayerReplicator>();

			base.OnUpdate(deltaTime);

			if (Input.IsKeyPressed(KeyCode.Space))
				LevelManager.Instance?.RevealAdjacentTilesInSandbox(Translation);
		}

		protected override void OnTargetReached()
		{
			m_InputEnabled = true;
		}

		protected override void OnDeath()
		{
			IsActivePlayer = true;
			m_InputEnabled = true;
		}

		protected override void Move(float ts)
		{
			if (!m_InputEnabled || !IsActivePlayer || LevelManager.Instance == null || !LevelManager.Instance.GameStarted)
				return;

			bool up = Input.IsKeyPressed(KeyCode.W) || Input.IsKeyPressed(KeyCode.Up);
			bool down = Input.IsKeyPressed(KeyCode.S) || Input.IsKeyPressed(KeyCode.Down);
			bool left = Input.IsKeyPressed(KeyCode.A) || Input.IsKeyPressed(KeyCode.Left);
			bool right = Input.IsKeyPressed(KeyCode.D) || Input.IsKeyPressed(KeyCode.Right);

			if (up && LevelManager.Instance.HasValidSandboxTile(Translation + Vector3.Forward))
			{
				m_TargetLocation = new Vector3Int(Translation + Vector3.Forward);
				m_Replicator?.RecordMove(-Vector3.Forward);
				m_InputEnabled = false;
				PlayAnimation(0);
				PlayMoveAudio();
			}
			else if (left && LevelManager.Instance.HasValidSandboxTile(Translation - Vector3.Right))
			{
				m_TargetLocation = new Vector3Int(Translation - Vector3.Right);
				m_Replicator?.RecordMove(Vector3.Right);
				m_InputEnabled = false;
				PlayAnimation(0);
				PlayMoveAudio();
			}
			else if (down && LevelManager.Instance.HasValidSandboxTile(Translation - Vector3.Forward))
			{
				m_TargetLocation = new Vector3Int(Translation - Vector3.Forward);
				m_Replicator?.RecordMove(Vector3.Forward);
				m_InputEnabled = false;
				PlayAnimation(0);
				PlayMoveAudio();
			}
			else if (right && LevelManager.Instance.HasValidSandboxTile(Translation + Vector3.Right))
			{
				m_TargetLocation = new Vector3Int(Translation + Vector3.Right);
				m_Replicator?.RecordMove(-Vector3.Right);
				m_InputEnabled = false;
				PlayAnimation(0);
				PlayMoveAudio();
			}
		}
	}
}
