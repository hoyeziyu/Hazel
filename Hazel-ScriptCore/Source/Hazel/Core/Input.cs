namespace Hazel
{
    public enum KeyCode
    {
        Space = 32,
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        Escape = 256,
        Enter = 257,
        Left = 263,
        Right = 262,
        Up = 265,
        Down = 264,
    }

	public enum MouseButton
	{
		Left = 0,
		Right = 1,
		Middle = 2,
	}

	public static class Input
	{
		public static bool IsKeyPressed(KeyCode keycode)
		{
			unsafe { return InternalCalls.Input_IsKeyPressed(keycode); }
		}

		public static bool IsMouseButtonPressed(MouseButton button)
		{
			unsafe { return InternalCalls.Input_IsMouseButtonPressed(button); }
		}

		public static Vector2 GetMousePosition()
		{
			Vector2 position;
			unsafe { InternalCalls.Input_GetMousePosition(&position); }
			return position;
		}
	}
}
