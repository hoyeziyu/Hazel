using Coral.Managed.Interop;

namespace Hazel
{
	internal static unsafe class InternalCalls
	{
#pragma warning disable CS0649

		internal static delegate* unmanaged<ulong, bool> Scene_IsEntityValid;

		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_GetTranslation;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_SetTranslation;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_GetRotation;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_SetRotation;

		internal static delegate* unmanaged<Log.LogLevel, NativeString, void> Log_LogMessage;

#pragma warning restore CS0649
	}
}
