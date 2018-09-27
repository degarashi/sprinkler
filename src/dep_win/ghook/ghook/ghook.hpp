#pragma once
#ifdef BUILD_DLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT __declspec(dllimport)
#endif

extern "C" {
	DLL_EXPORT bool IsHooking();
	DLL_EXPORT bool InitHook();
	DLL_EXPORT void ExitHook();
	DLL_EXPORT void Stop();
	DLL_EXPORT void Wait();
}
