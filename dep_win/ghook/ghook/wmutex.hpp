#pragma once
#include <windows.h>

class WMutex {
	private:
		HANDLE	_mutex;
	public:
		WMutex(LPCTSTR name);
		HANDLE ref() const noexcept;
		~WMutex() noexcept;
		void lock();
		void unlock() noexcept;
};
