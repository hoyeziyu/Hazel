#pragma once

namespace Hazel {

	class Time
	{
	public:
		static float GetTimeScale() { return s_TimeScale; }
		static void SetTimeScale(float timeScale) { s_TimeScale = timeScale; }
		static void Reset() { s_TimeScale = 1.0f; }

	private:
		static inline float s_TimeScale = 1.0f;
	};

}
