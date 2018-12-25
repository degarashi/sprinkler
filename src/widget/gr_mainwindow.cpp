#include "gr_mainwindow.hpp"

namespace dg::widget {
	void GeomRestore_MW::_onVisibilityChanged(const bool b) {
		emit onVisibilityChanged(b);
	}
}
