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

		internal static delegate* unmanaged<ulong, uint> AnimationComponent_GetStateIndex;
		internal static delegate* unmanaged<ulong, uint, void> AnimationComponent_SetStateIndex;
		internal static delegate* unmanaged<ulong, float> AnimationComponent_GetAnimationTime;
		internal static delegate* unmanaged<ulong, float, void> AnimationComponent_SetAnimationTime;
		internal static delegate* unmanaged<ulong, bool> AnimationComponent_GetIsAnimationPlaying;
		internal static delegate* unmanaged<ulong, bool, void> AnimationComponent_SetIsAnimationPlaying;

		internal static delegate* unmanaged<ulong, ulong> Entity_GetParent;
		internal static delegate* unmanaged<ulong, ulong, void> Entity_SetParent;
		internal static delegate* unmanaged<ulong, NativeArray<ulong>> Entity_GetChildren;
		internal static delegate* unmanaged<ulong, ReflectionType, bool> Entity_HasComponent;

		internal static delegate* unmanaged<ulong, NativeInstance<object>> ScriptComponent_GetInstance;

		internal static delegate* unmanaged<ulong, bool> AudioComponent_GetPlayOnAwake;
		internal static delegate* unmanaged<ulong, bool, void> AudioComponent_SetPlayOnAwake;
		internal static delegate* unmanaged<ulong, float> AudioComponent_GetVolume;
		internal static delegate* unmanaged<ulong, float, void> AudioComponent_SetVolume;

		internal static delegate* unmanaged<NativeString, uint> AudioCommandID_Constructor;
		internal static delegate* unmanaged<uint, ulong, uint> Audio_PostEventFromAC;

		internal static delegate* unmanaged<ulong, ulong> Scene_InstantiatePrefab;
		internal static delegate* unmanaged<ulong, Vector3*, ulong> Scene_InstantiatePrefabWithTranslation;
		internal static delegate* unmanaged<ulong, Vector3*, Vector3*, Vector3*, ulong> Scene_InstantiatePrefabWithTransform;
		internal static delegate* unmanaged<ulong, ulong, Vector3*, ulong> Scene_InstantiateChildPrefabWithTranslation;
		internal static delegate* unmanaged<ulong, ulong, Vector3*, Vector3*, Vector3*, ulong> Scene_InstantiateChildPrefabWithTransform;

		internal static delegate* unmanaged<ulong, float> CameraComponent_GetVerticalFOV;
		internal static delegate* unmanaged<ulong, float, void> CameraComponent_SetVerticalFOV;

		internal static delegate* unmanaged<int, NativeString, void> RuntimeHUD_SetLine;
		internal static delegate* unmanaged<void> RuntimeHUD_Clear;

		internal static delegate* unmanaged<float> Time_GetTimeScale;
		internal static delegate* unmanaged<float, void> Time_SetTimeScale;

#pragma warning restore CS0649
	}
}
