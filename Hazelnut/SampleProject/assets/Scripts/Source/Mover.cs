using Hazel;

namespace Sample
{
	public class Mover : Entity
	{
		private float m_Speed = 4.0f;

		protected override void OnUpdate(float ts)
		{
			Vector3 delta = Vector3.Zero;

			if (Input.IsKeyPressed(KeyCode.W))
				delta.Y += 1.0f;
			if (Input.IsKeyPressed(KeyCode.S))
				delta.Y -= 1.0f;
			if (Input.IsKeyPressed(KeyCode.A))
				delta.X -= 1.0f;
			if (Input.IsKeyPressed(KeyCode.D))
				delta.X += 1.0f;

			if (delta.X != 0.0f || delta.Y != 0.0f)
				Translation += delta * m_Speed * ts;
		}
	}
}
