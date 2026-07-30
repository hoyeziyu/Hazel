using Coral.Managed.Interop;

namespace Hazel
{
	public static class HUD
	{
		public static void SetLine(int index, string text)
		{
			unsafe { InternalCalls.RuntimeHUD_SetLine(index, text); }
		}

		public static void Clear()
		{
			unsafe { InternalCalls.RuntimeHUD_Clear(); }
		}
	}
}
