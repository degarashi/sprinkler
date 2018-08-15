#include "x_aux.hpp"
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <iostream>
#include <cassert>

namespace dg {
	namespace x_deleter {
		void Display::operator()(_XDisplay* d) const noexcept {
			::XCloseDisplay(d);
		}
		struct WindowV {
			void operator()(Window* p) const noexcept {
				::XFree(p);
			}
		};
		struct StringList {
			void operator()(char** p) const noexcept {
				::XFreeStringList(p);
			}
		};
	}
	namespace {
		using WindowV_U = std::unique_ptr<Window[], x_deleter::WindowV>;
		using StringL_U = std::unique_ptr<char*[], x_deleter::StringList>;
	}

	std::optional<Name> GetWindowName(_XDisplay* disp, const WindowD wd) {
		const Window w = wd.as<Window>();
		const Atom atom_name = XInternAtom(disp, "_NET_WM_NAME", true);
		XTextProperty textprop;
		auto status = XGetTextProperty(disp, w, &textprop, atom_name);
		bool valid = true;
		if(status==0 || textprop.nitems==0 || !textprop.value)
			status = XGetWMName(disp, w, &textprop);
		if(status==0 || textprop.nitems==0 || !textprop.value)
			valid = false;

		if(valid) {
			char** list_p;
			int count;
			status = XmbTextPropertyToTextList(disp, &textprop, &list_p, &count);
			if(status == 0) {
				assert(list_p);
				StringL_U list(list_p);
				return list[0];
			}
		}
		return std::nullopt;
	}
	bool IsWindowViewable(_XDisplay* disp, const WindowD wd) {
		const Window w = wd.as<Window>();
		XWindowAttributes attr;
		XGetWindowAttributes(disp, w, &attr);
		return attr.map_state == IsViewable;
	}
	WindowD SelectWindow(_XDisplay* disp, const WindowD rootd) {
		const Window root = rootd.as<Window>();
		const auto cursor = XCreateFontCursor(disp, XC_crosshair);
		auto status = XGrabPointer(
			disp, root, False,
			ButtonPressMask|ButtonReleaseMask,
			GrabModeSync, GrabModeAsync,
			root, cursor, CurrentTime
		);
		if(status != GrabSuccess)
			throw std::runtime_error("Can't grab the mouse.");

		std::optional<Window> result;
		int buttons = 0;
		while(!result || (buttons != 0)) {
			XAllowEvents(disp, SyncPointer, CurrentTime);

			XEvent e;
			XWindowEvent(disp, root, ButtonPressMask|ButtonReleaseMask, &e);
			if(e.type == ButtonPress) {
				if(!result) {
					auto& btn = e.xbutton;
					if(btn.subwindow != None)
						result = btn.subwindow;
					else
						result = btn.window;
				}
				++buttons;
				// キャンセル時はNone
			} else if(e.type == ButtonRelease) {
				if(buttons > 0)
					--buttons;
			}
		}
		XUngrabPointer(disp, CurrentTime);
		XFreeCursor(disp, cursor);
		return *result;
	}
}

#include "clientwin.h"
namespace dg {
	WindowD SelectTopWindow(_XDisplay* disp, const WindowD rootd) {
		const Window root = rootd.as<Window>();
		return Find_Client(disp, root, SelectWindow(disp, root).as<Window>());
	}
	bool IsClientWindow(_XDisplay* disp, const WindowD rootd,  const WindowD wd) {
		const Window root = rootd.as<Window>(),
					w = wd.as<Window>();
		return Find_Client(disp, root, w) == w;
	}
	bool EnumerateTopWindow(_XDisplay* disp, const WindowD rootd, const WindowD wd, const bool viewableonly,
			const std::function<bool (Window, const dg::Name&)>& cb)
	{
		const Window root = rootd.as<Window>(),
						w = wd.as<Window>();
		if(IsClientWindow(disp, root, w)) {
			if(const auto name = dg::GetWindowName(disp, w))
				return !cb(w, *name);
		}

		WindowV_U children;
		unsigned int numChildren;
		{
			Window root,
				   *children_p,
				   parent;
			if(!XQueryTree(disp, w, &root, &parent, &children_p, &numChildren)) {
				return false;
			}
			children.reset(children_p);
		}
		if(children) {
			for(unsigned int i=0 ; i<numChildren ; i++) {
				const auto& c = children[i];
				// 表示されている物だけが対象
				if(!viewableonly || dg::IsWindowViewable(disp, c)) {
					// search lower layer
					if(EnumerateTopWindow(disp, root, c, viewableonly, cb))
						return true;
				}
			}
		} else {
			assert(numChildren == 0);
		}
		return false;
	}
}
