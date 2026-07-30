using System.Runtime.InteropServices;

namespace Hazel
{
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct Transform
	{
		public Vector3 Position;
		public Vector3 Rotation;
		public Vector3 Scale;
	}
}
