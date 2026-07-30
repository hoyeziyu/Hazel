using System;

using Hazel;

namespace LD51
{
	public struct Vector3Int
	{
		public int X;
		public int Y;
		public int Z;

		public Vector3Int(int x, int y, int z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public Vector3Int(Vector3 vec)
		{
			X = (int)Math.Round(vec.X);
			Y = (int)Math.Round(vec.Y);
			Z = (int)Math.Round(vec.Z);
		}

		public Vector3 ToVec3() => new Vector3(X, Y, Z);

		public override string ToString() => $"Vector3Int({X}, {Y}, {Z})";
	}
}
