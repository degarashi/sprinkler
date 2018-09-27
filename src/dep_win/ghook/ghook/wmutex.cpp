#include "wmutex.hpp"
#include <stdexcept>

WMutex::WMutex(LPCTSTR name):
	_mutex(CreateMutex(NULL, FALSE, name))
{
	if(!_mutex)
		throw std::runtime_error("something wrong(WMutex())");
}
HANDLE WMutex::ref() const noexcept {
	return _mutex;
}
WMutex::~WMutex() noexcept {
	CloseHandle(_mutex);
}
void WMutex::lock() {
	switch(WaitForSingleObject(_mutex, INFINITE)) {
		case WAIT_ABANDONED:
		case WAIT_OBJECT_0:
			break;
		case WAIT_TIMEOUT:
		case WAIT_FAILED:
			throw std::runtime_error("something wrong(WMutex::lock())");
		default:
			break;
	}
}
void WMutex::unlock() noexcept {
	ReleaseMutex(_mutex);
}
