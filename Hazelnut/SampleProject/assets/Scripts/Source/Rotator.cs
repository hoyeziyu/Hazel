using Hazel;

namespace Sample
{
	public class Rotator : Entity
	{
		private float m_Speed = 1.5f;

		protected override void OnCreate()
		{
			Log.Info("Rotator attached to entity {0}", ID);
		}

		protected override void OnUpdate(float ts)
		{
			Rotation += new Vector3(0.0f, m_Speed * ts, 0.0f);
		}
	}
}
