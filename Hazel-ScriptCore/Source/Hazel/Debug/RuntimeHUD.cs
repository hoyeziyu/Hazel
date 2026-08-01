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

		public static void SetWorldLabel(int index, Vector3 worldPosition, string text)
		{
			unsafe { InternalCalls.RuntimeWorldLabel_Set(index, &worldPosition, text); }
		}

		public static void ClearWorldLabels()
		{
			unsafe { InternalCalls.RuntimeWorldLabel_Clear(); }
		}
	}
}
