#include "gr_widget.hpp"

namespace dg::widget {
	void GeomRestore_Widget::_onVisibilityChanged(const bool b) {
		emit onVisibilityChanged(b);
	}
}
