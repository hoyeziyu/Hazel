using Coral.Managed.Interop;

namespace Hazel
{
	internal static unsafe class InternalCalls
	{
#pragma warning disable CS0649

		internal static delegate* unmanaged<ulong, bool> Scene_IsEntityValid;
		internal static delegate* unmanaged<NativeString, ulong> Scene_FindEntityByTag;
		internal static delegate* unmanaged<NativeString, ulong> Scene_CreateEntity;
		internal static delegate* unmanaged<ulong, void> Scene_DestroyEntity;
		internal static delegate* unmanaged<NativeArray<ulong>> Scene_GetEntities;

		internal static delegate* unmanaged<ulong, NativeString> TagComponent_GetTag;
		internal static delegate* unmanaged<ulong, NativeString, void> TagComponent_SetTag;

		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_GetTranslation;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_SetTranslation;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_GetRotation;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_SetRotation;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_GetScale;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_SetScale;

		internal static delegate* unmanaged<KeyCode, bool> Input_IsKeyPressed;
		internal static delegate* unmanaged<MouseButton, bool> Input_IsMouseButtonPressed;
		internal static delegate* unmanaged<Vector2*, void> Input_GetMousePosition;

		internal static delegate* unmanaged<ulong, int> RigidBody2DComponent_GetBodyType;
		internal static delegate* unmanaged<ulong, int, void> RigidBody2DComponent_SetBodyType;
		internal static delegate* unmanaged<ulong, Vector2*, void> RigidBody2DComponent_GetLinearVelocity;
		internal static delegate* unmanaged<ulong, Vector2*, void> RigidBody2DComponent_SetLinearVelocity;
		internal static delegate* unmanaged<ulong, float> RigidBody2DComponent_GetGravityScale;
		internal static delegate* unmanaged<ulong, float, void> RigidBody2DComponent_SetGravityScale;
		internal static delegate* unmanaged<ulong, Vector2*, Vector2*, bool, void> RigidBody2DComponent_ApplyLinearImpulse;

		internal static delegate* unmanaged<Log.LogLevel, NativeString, void> Log_LogMessage;

#pragma warning restore CS0649
	}
}
