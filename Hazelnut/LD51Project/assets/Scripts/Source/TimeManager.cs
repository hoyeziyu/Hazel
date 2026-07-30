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
		private int m_LastLoggedSecond = -1;

		protected override void OnCreate()
		{
			m_Timer = RoundDuration;
		}

		public void EnterSlowMotion() { }
		public void ExitSlowMotion() { }

		protected override void OnUpdate(float ts)
		{
			m_LevelManager ??= FindEntityByTag("LevelManager")?.As<LevelManager>();
			if (m_LevelManager == null || !m_LevelManager.GameStarted)
				return;

			m_Timer -= ts;
			int secondsLeft = Mathf.CeilToInt(m_Timer);
			if (secondsLeft != m_LastLoggedSecond && secondsLeft <= 9)
			{
				m_LastLoggedSecond = secondsLeft;
				Log.Info("TimeManager: {0}s", secondsLeft);
			}

			if (m_Timer <= 0.0f)
			{
				m_LevelManager.SwitchRound();
				m_Timer = RoundDuration;
				m_LastLoggedSecond = -1;
				Log.Info("TimeManager: switch round");
			}
		}

		public void Reset()
		{
			m_Timer = RoundDuration;
			m_LastLoggedSecond = -1;
		}
	}
}
