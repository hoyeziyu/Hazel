using System;

using Hazel;

namespace LD51
{
	public class TimeManager : Entity
	{
		public Entity CameraEntity = null!;
		public Entity PlayerEntity = null!;
		public Entity PlayerReplicatorEntity = null!;

		public float RoundDuration = 10.0f;

		private float m_Timer;
		private LevelManager? m_LevelManager;
		private bool m_InSlowMotion;
		private float m_DefaultFOV = 45.0f;

		protected override void OnCreate()
		{
			m_Timer = RoundDuration;
			if (CameraEntity != null && CameraEntity.HasComponent<CameraComponent>())
				m_DefaultFOV = CameraEntity.GetComponent<CameraComponent>()!.VerticalFOV;
		}

		public void EnterSlowMotion()
		{
			if (m_InSlowMotion)
				return;

			m_InSlowMotion = true;
			Time.TimeScale = 0.25f;

			if (CameraEntity != null && CameraEntity.HasComponent<CameraComponent>())
			{
				var camera = CameraEntity.GetComponent<CameraComponent>()!;
				m_DefaultFOV = camera.VerticalFOV;
				camera.VerticalFOV = m_DefaultFOV * 0.75f;
			}
		}

		public void ExitSlowMotion()
		{
			if (!m_InSlowMotion)
				return;

			m_InSlowMotion = false;
			Time.TimeScale = 1.0f;

			if (CameraEntity != null && CameraEntity.HasComponent<CameraComponent>())
				CameraEntity.GetComponent<CameraComponent>()!.VerticalFOV = m_DefaultFOV;
		}

		protected override void OnUpdate(float ts)
		{
			m_LevelManager ??= FindEntityByTag("LevelManager")?.As<LevelManager>();
			if (m_LevelManager == null || !m_LevelManager.GameStarted)
				return;

			m_Timer -= ts;
			int secondsLeft = Math.Max(0, Mathf.CeilToInt(m_Timer));
			string phase = m_LevelManager.IsReplicatingMoves() ? "Replicator" : "Player";
			HUD.SetLine(0, $"{secondsLeft}s");
			HUD.SetLine(1, phase);
			HUD.SetLine(2, m_InSlowMotion ? "Slow Motion (F)" : "Hold F: Slow Motion");

			if (m_Timer <= 0.0f)
			{
				ExitSlowMotion();
				m_LevelManager.SwitchRound();
				m_Timer = RoundDuration;
				Log.Info("TimeManager: switch round");
			}
		}

		public void Reset()
		{
			ExitSlowMotion();
			m_Timer = RoundDuration;
			HUD.Clear();
		}
	}
}
