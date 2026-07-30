using System;
using System.Runtime.InteropServices;

namespace Hazel
{
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct Vector3 : IEquatable<Vector3>
	{
		public static Vector3 Zero = new Vector3(0, 0, 0);
		public static Vector3 One = new Vector3(1, 1, 1);
		public static Vector3 Forward = new Vector3(0, 0, -1);
		public static Vector3 Back = new Vector3(0, 0, 1);
		public static Vector3 Right = new Vector3(1, 0, 0);
		public static Vector3 Left = new Vector3(-1, 0, 0);
		public static Vector3 Up = new Vector3(0, 1, 0);
		public static Vector3 Down = new Vector3(0, -1, 0);

		public float X;
		public float Y;
		public float Z;

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public Vector3(float x, Vector2 yz)
		{
			X = x;
			Y = yz.X;
			Z = yz.Y;
		}

		public Vector2 XY
		{
			get => new Vector2(X, Y);
			set { X = value.X; Y = value.Y; }
		}

		public Vector2 XZ
		{
			get => new Vector2(X, Z);
			set { X = value.X; Z = value.Y; }
		}

		public Vector2 YZ
		{
			get => new Vector2(Y, Z);
			set { Y = value.X; Z = value.Y; }
		}

		public float Length() => (float)Math.Sqrt(X * X + Y * Y + Z * Z);

		public Vector3 Normalized()
		{
			float length = Length();
			return length > 0.0f ? this * (1.0f / length) : this;
		}

		public static float Distance(Vector3 p1, Vector3 p2)
		{
			float dx = p2.X - p1.X;
			float dy = p2.Y - p1.Y;
			float dz = p2.Z - p1.Z;
			return (float)Math.Sqrt(dx * dx + dy * dy + dz * dz);
		}

		public static Vector3 Lerp(Vector3 p1, Vector3 p2, float t) => p1 + (p2 - p1) * t;

		public static Vector3 operator +(Vector3 a, Vector3 b) => new(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
		public static Vector3 operator -(Vector3 a, Vector3 b) => new(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
		public static Vector3 operator -(Vector3 v) => new(-v.X, -v.Y, -v.Z);
		public static Vector3 operator *(Vector3 v, float s) => new(v.X * s, v.Y * s, v.Z * s);
		public static Vector3 operator /(Vector3 v, float s) => new(v.X / s, v.Y / s, v.Z / s);

		public bool Equals(Vector3 other) => X == other.X && Y == other.Y && Z == other.Z;
		public override bool Equals(object? obj) => obj is Vector3 other && Equals(other);
		public override int GetHashCode() => HashCode.Combine(X, Y, Z);
	}
}
