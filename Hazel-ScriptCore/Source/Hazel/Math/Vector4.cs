using System;
using System.Runtime.InteropServices;

namespace Hazel
{
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct Vector4 : IEquatable<Vector4>
	{
		public static Vector4 Zero = new Vector4(0, 0, 0, 0);
		public static Vector4 One = new Vector4(1, 1, 1, 1);

		public float X;
		public float Y;
		public float Z;
		public float W;

		public Vector4(float x, float y, float z, float w)
		{
			X = x;
			Y = y;
			Z = z;
			W = w;
		}

		public Vector3 XYZ
		{
			get => new Vector3(X, Y, Z);
			set { X = value.X; Y = value.Y; Z = value.Z; }
		}

		public bool Equals(Vector4 other) => X == other.X && Y == other.Y && Z == other.Z && W == other.W;
		public override bool Equals(object? obj) => obj is Vector4 other && Equals(other);
		public override int GetHashCode() => HashCode.Combine(X, Y, Z, W);
	}
}
