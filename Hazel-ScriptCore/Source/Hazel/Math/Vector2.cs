using System;
using System.Runtime.InteropServices;

namespace Hazel
{
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct Vector2 : IEquatable<Vector2>
	{
		public float X;
		public float Y;

		public Vector2(float x, float y)
		{
			X = x;
			Y = y;
		}

		public static Vector2 Zero => new(0, 0);

		public bool Equals(Vector2 other) => X == other.X && Y == other.Y;
		public override bool Equals(object? obj) => obj is Vector2 other && Equals(other);
		public override int GetHashCode() => HashCode.Combine(X, Y);
	}
}
