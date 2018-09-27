#pragma once
#include <windows.h>

class WEvent {
	private:
		HANDLE _event;
	public:
		WEvent(LPCTSTR name);
		void signal();
		void reset();
		void pulse();
		void wait();
		HANDLE ref() const noexcept;
		~WEvent();
};
