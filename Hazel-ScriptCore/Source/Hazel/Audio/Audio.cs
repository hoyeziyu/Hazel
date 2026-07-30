using System.Runtime.InteropServices;

namespace Hazel
{
	[StructLayout(LayoutKind.Sequential)]
	public struct AudioCommandID
	{
		public readonly uint ID;

		public AudioCommandID(string commandName)
		{
			unsafe { ID = InternalCalls.AudioCommandID_Constructor(commandName); }
		}

		public static bool operator ==(AudioCommandID c1, AudioCommandID c2) => c1.ID == c2.ID;
		public static bool operator !=(AudioCommandID c1, AudioCommandID c2) => c1.ID != c2.ID;
		public override bool Equals(object? c) => c is AudioCommandID other && ID == other.ID;
		public override int GetHashCode() => ID.GetHashCode();
		public static implicit operator uint(AudioCommandID commandID) => commandID.ID;
	}

	public static class Audio
	{
		public static uint PostEvent(AudioCommandID id, ulong objectID)
		{
			unsafe { return InternalCalls.Audio_PostEventFromAC(id.ID, objectID); }
		}

		public static uint PostEvent(string eventName, ulong objectID) => PostEvent(new AudioCommandID(eventName), objectID);

		public static uint PostEvent(AudioCommandID id, ref AudioComponent audioComponent)
		{
			unsafe { return InternalCalls.Audio_PostEventFromAC(id.ID, audioComponent.Entity.ID); }
		}
	}
}
