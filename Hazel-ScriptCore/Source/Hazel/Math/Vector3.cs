using System;
using System.Runtime.InteropServices;

namespace Hazel
{
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct Vector3 : IEquatable<Vector3>
	{
		public float X;
		public float Y;
		public float Z;

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public static Vector3 operator +(Vector3 a, Vector3 b) => new(a.X + b.X, a.Y + b.Y, a.Z + b.Z);

		public bool Equals(Vector3 other) => X == other.X && Y == other.Y && Z == other.Z;
		public override bool Equals(object? obj) => obj is Vector3 other && Equals(other);
		public override int GetHashCode() => HashCode.Combine(X, Y, Z);
	}
}
