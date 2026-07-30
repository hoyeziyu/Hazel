namespace Hazel
{
	public static class Time
	{
		public static float TimeScale
		{
			get { unsafe { return InternalCalls.Time_GetTimeScale(); } }
			set { unsafe { InternalCalls.Time_SetTimeScale(value); } }
		}
	}
}
