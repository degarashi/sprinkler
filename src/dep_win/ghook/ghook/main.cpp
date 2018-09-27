#include <windows.h>
#include <iostream>
#include <memory>
#include <mutex>
#include "wevent.hpp"
#include "ghook.hpp"
#include <tchar.h>

#define SHARE __attribute__((section ("degarashi_seg"), shared))
#define _STRINGIZE(s) #s
#define STRINGIZE(s) _STRINGIZE(s)
struct SyncValue {
	WEvent	event;

	SyncValue():
		event(_T("Wait") _T(STRINGIZE(BITVERSION)) _T("_Signal"))
	{}
};
std::unique_ptr<SyncValue>	g_sync;

HINSTANCE g_hDLLMod = NULL;
extern "C" {
	SHARE HHOOK g_hkShell = NULL;
	SHARE HHOOK g_hkMsg = NULL;
}

#define DEF_PAIR(name)	{name, #name}
namespace param_name {
	const std::pair<int, const char*> shell[] = {
		DEF_PAIR(HSHELL_ACCESSIBILITYSTATE),
		DEF_PAIR(HSHELL_ACTIVATESHELLWINDOW),
		DEF_PAIR(HSHELL_APPCOMMAND),
		DEF_PAIR(HSHELL_GETMINRECT),
		DEF_PAIR(HSHELL_LANGUAGE),
		DEF_PAIR(HSHELL_REDRAW),
		DEF_PAIR(HSHELL_TASKMAN),
		DEF_PAIR(HSHELL_WINDOWACTIVATED),
		DEF_PAIR(HSHELL_WINDOWCREATED),
		DEF_PAIR(HSHELL_WINDOWDESTROYED)
	};
	template <class T, size_t N, class Id>
	const char* Find(const T (&list)[N], const Id id) {
		for(size_t i=0 ; i<sizeof(list)/sizeof(list[0]) ; i++) {
			if(list[i].first == id)
				return list[i].second;
		}
		return nullptr;
	}
}
#undef DEF_PAIR

LRESULT CALLBACK ShellProc(const int code, const WPARAM wp, const LPARAM lp) {
	// std::cout << "ShellProc: " << wp << ", " << lp << std::endl;
	if(code < 0 || code == HC_NOREMOVE) {}
	else {
		if(code == HSHELL_WINDOWCREATED ||
			code == HSHELL_WINDOWDESTROYED)
		{
			HWND hw = reinterpret_cast<HWND>(wp);
			std::cout << param_name::Find(param_name::shell, code) << ": " << hw << std::endl;

			// char buf[MAX_PATH];
			// GetModuleFileName(NULL, buf, sizeof(buf));
			// std::cout << buf << std::endl;

			g_sync->event.signal();
		}
	}
	return CallNextHookEx(g_hkShell, code, wp, lp);
}
LRESULT CALLBACK WndProc(const int code, const WPARAM wp, const LPARAM lp) {
	if(code < 0 || code == HC_NOREMOVE) {}
	else {
		auto* p = reinterpret_cast<const CWPSTRUCT*>(lp);
		if(p->message == WM_MOVE) {
			std::cout << "WM_MOVE: " << p->hwnd << std::endl;
			g_sync->event.signal();
		}
		if(p->message == WM_SIZE) {
			std::cout << "WM_SIZE: " << p->hwnd << std::endl;
			g_sync->event.signal();
		}
		// char buf[256];
		// HWND w = p->hwnd;
		// do {
			// GetWindowText(w, buf, sizeof(buf));
			// std::cout << buf << std::endl;
			// w = GetParent(w);
		// } while(w);
	}
	return CallNextHookEx(g_hkMsg, code, wp, lp);
}
extern "C" {
	bool IsHooking() {
		return static_cast<bool>(g_hkShell) &&
				static_cast<bool>(g_hkMsg);
	}
	bool InitHook() {
		std::cout << "InitHook()" << std::endl;
		if(!g_hkShell) {
			g_hkShell = SetWindowsHookEx(WH_SHELL, static_cast<HOOKPROC>(ShellProc), g_hDLLMod, 0);
			g_hkMsg = SetWindowsHookEx(WH_CALLWNDPROC, static_cast<HOOKPROC>(WndProc), g_hDLLMod, 0);
			return IsHooking();
		}
		return false;
	}
	void ExitHook() {
		std::cout << "ExitHook()" << std::endl;
		if(g_hkShell) {
			UnhookWindowsHookEx(g_hkShell);
			UnhookWindowsHookEx(g_hkMsg);
			g_hkShell = NULL;
			g_hkMsg = NULL;
		}
	}
	void Stop() {
		g_sync->event.signal();
	}
	void Wait() {
		g_sync->event.wait();
	}

	BOOL WINAPI DllMain(HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved) {
		switch(dwReason) {
			case DLL_PROCESS_ATTACH:
				std::cout << "DLL_PROCESS_ATTACH: " << BITVERSION << std::endl;
				g_hDLLMod = hDll;
				g_sync.reset(new SyncValue);
				break;
			case DLL_THREAD_ATTACH:
				std::cout << "DLL_THREAD_ATTACH: " << BITVERSION << std::endl;
				break;
			case DLL_THREAD_DETACH:
				std::cout << "DLL_THREAD_DETACH: " << BITVERSION << std::endl;
				break;
			case DLL_PROCESS_DETACH:
				std::cout << "DLL_PROCESS_DETACH: " << BITVERSION << std::endl;
				g_sync.reset();
				break;
			default:
				break;
		}
		return TRUE;
	}
}
