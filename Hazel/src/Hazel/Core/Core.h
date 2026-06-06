#pragma once

#ifdef HZ_PLATFORM_WINDOWS
	#ifdef HZ_BUILD_DLL
		#define HAZEL_API __declspec(dllexport)
	#elif defined(HZ_DLL)
		#define HAZEL_API __declspec(dllimport)
	#else
		#define HAZEL_API
	#endif
#else
	#error Hazel only supports Windows!
#endif