#pragma once
#include <memory>
#include <optional>
#include <functional>
#include "windowd.hpp"

extern "C" {
	struct _XDisplay;
}
namespace dg {
	namespace x_deleter {
		struct Display {
			void operator()(_XDisplay* d) const noexcept;
		};
	}
	using Display_U = std::unique_ptr<_XDisplay, x_deleter::Display>;

	using Name = std::string;
	std::optional<Name> GetWindowName(_XDisplay* disp, WindowD w);
	bool IsWindowViewable(_XDisplay* disp, WindowD w);
	WindowD SelectWindow(_XDisplay* disp, WindowD root);
	WindowD SelectTopWindow(_XDisplay* disp, WindowD root);

	bool IsClientWindow(_XDisplay* disp, WindowD root,  WindowD w);
	bool EnumerateTopWindow(_XDisplay* disp, WindowD root, WindowD w, bool viewableonly, const std::function<bool (WindowD, const dg::Name&)>& cb);
}
