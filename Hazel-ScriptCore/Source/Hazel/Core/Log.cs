using Coral.Managed.Interop;

namespace Hazel
{
	public static class Log
	{
		internal enum LogLevel
		{
			Trace = 1 << 0,
			Debug = 1 << 1,
			Info = 1 << 2,
			Warn = 1 << 3,
			Error = 1 << 4,
			Critical = 1 << 5
		}

		public static void Trace(string format, params object[] parameters)
		{
			NativeString message = FormatUtils.Format(format, parameters);
			unsafe { InternalCalls.Log_LogMessage(LogLevel.Trace, message); }
		}

		public static void Info(string format, params object[] parameters)
		{
			NativeString message = FormatUtils.Format(format, parameters);
			unsafe { InternalCalls.Log_LogMessage(LogLevel.Info, message); }
		}

		public static void Warn(string format, params object[] parameters)
		{
			NativeString message = FormatUtils.Format(format, parameters);
			unsafe { InternalCalls.Log_LogMessage(LogLevel.Warn, message); }
		}

		public static void Error(string format, params object[] parameters)
		{
			NativeString message = FormatUtils.Format(format, parameters);
			unsafe { InternalCalls.Log_LogMessage(LogLevel.Error, message); }
		}
	}
}
