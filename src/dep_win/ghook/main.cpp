#include "names.hpp"
#include "ghook/ghook.hpp"
#include "ghook/wmutex.hpp"
#include "ghook/wevent.hpp"
#include <tchar.h>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <mutex>

LRESULT CALLBACK WndProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hw, msg, wp, lp);
}

std::unique_ptr<WMutex> g_mutex;
std::unique_ptr<WEvent> g_event;
bool g_loop = false;

DWORD WINAPI ThreadFunc(LPVOID param) {
	static int count = 0;
	UNREFERENCED_PARAMETER(param);
	for(;;) {
		Wait();
		g_event->signal();
		std::cout << "ThreadFunc: Signal " << count++ << std::endl;

		std::lock_guard lk(*g_mutex);
		if(!g_loop)
			break;
	}
	std::cout << "ThreadFunc: Exiting..." << std::endl;
	return 0;
}

int _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmd, int cmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmd);
	UNREFERENCED_PARAMETER(cmdShow);

	try {
		if(IsHooking()) {
			throw std::runtime_error("Instance duplication detected.\nThis application allowed only single instance at once");
		}
		InitHook();
		g_mutex = std::make_unique<WMutex>(name::Mutex);
		g_event = std::make_unique<WEvent>(name::Event);

		WNDCLASSEX wc = {};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpfnWndProc = WndProc;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = name::WndClass;

		if(!RegisterClassEx(&wc))
			throw std::runtime_error("Can't register window class");

		const HWND hw = CreateWindowEx(
			WS_EX_TOOLWINDOW,
			name::WndClass, name::WndTitle,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			256, 128,
			NULL, NULL,
			hInstance, NULL
		);
		if(!hw)
			throw std::runtime_error("Can't create window");

		g_loop = true;
		DWORD id;
		const HANDLE hTh = CreateThread(NULL, 0, ThreadFunc, reinterpret_cast<LPVOID>(hw), 0, &id);
		if(!hTh)
			throw std::runtime_error("Can't create thread");

		MSG msg;
		while(GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		{
			std::lock_guard lk(*g_mutex);
			g_loop = false;
		}
		Stop();
		WaitForSingleObject(hTh, INFINITE);
		ExitHook();

		g_mutex.reset();
		g_event.reset();
	} catch (const std::exception& e) {
		MessageBox(NULL, e.what(), _T("Error"), MB_OK|MB_ICONWARNING);
		return 1;
	}
	return 0;
}
