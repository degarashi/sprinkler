#include "wevent.hpp"
#include <stdexcept>

WEvent::WEvent(LPCTSTR name):
	_event(CreateEvent(NULL, FALSE, FALSE, name))
{
	if(!_event)
		throw std::runtime_error("something wrong(WEvent())");
}
void WEvent::signal() {
	if(!SetEvent(_event))
		throw std::runtime_error("something wrong(WEvent::signal())");
}
void WEvent::reset() {
	if(!ResetEvent(_event))
		throw std::runtime_error("something wrong(WEvent::reset())");
}
void WEvent::pulse() {
	if(!PulseEvent(_event))
		throw std::runtime_error("something wrong(WEvent::pulse())");
}
void WEvent::wait() {
	switch(WaitForSingleObject(_event, INFINITE)) {
		case WAIT_ABANDONED:
		case WAIT_OBJECT_0:
			break;
		case WAIT_TIMEOUT:
		case WAIT_FAILED:
			throw std::runtime_error("something wrong(WEvent::wait())");
		default:
			break;
	}
}
HANDLE WEvent::ref() const noexcept {
	return _event;
}
WEvent::~WEvent() {
	CloseHandle(_event);
}
