using System;

namespace Hazel
{
	public static class Mathf
	{
		public const float Epsilon = 0.00001f;
		public const float PI = (float)Math.PI;
		public const float PIonTwo = (float)(Math.PI / 2.0f);
		public const float TwoPI = (float)(Math.PI * 2.0);
		public const float Deg2Rad = PI / 180.0f;
		public const float Rad2Deg = 180.0f / PI;

		public static float Sin(float value) => (float)Math.Sin(value);
		public static float Cos(float value) => (float)Math.Cos(value);
		public static float Tan(float value) => (float)Math.Tan(value);

		public static float Clamp(float value, float min, float max)
		{
			if (value < min)
				return min;
			return value > max ? max : value;
		}

		public static int Clamp(int value, int min, int max)
		{
			if (value < min)
				return min;
			return value > max ? max : value;
		}

		public static float Min(float v0, float v1) => v0 < v1 ? v0 : v1;
		public static float Max(float v0, float v1) => v0 > v1 ? v0 : v1;
		public static float Abs(float value) => Math.Abs(value);
		public static float Floor(float value) => (float)Math.Floor(value);
		public static int CeilToInt(float value) => (int)Math.Ceiling(value);
		public static int FloorToInt(float value) => (int)Math.Floor(value);
		public static float Lerp(float p1, float p2, float t) => p1 + (p2 - p1) * t;
		public static Vector3 Lerp(Vector3 p1, Vector3 p2, float t) => p1 + (p2 - p1) * t;
		public static float Distance(float p1, float p2) => Abs(p1 - p2);
	}
}
