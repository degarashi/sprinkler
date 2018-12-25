#include "cur_indicator.hpp"

namespace dg { namespace widget {
	void CurIndicator::_makeText() {
		setText(QString(tr("<b>%1</b> images (out of <b>%2</b>) were displayed")).arg(_cur).arg(_num));
	}
	void CurIndicator::onSprinkleCounterChanged(const size_t shown, const size_t notshown) {
		_cur = shown;
		_num = shown + notshown;
		_makeText();
		update();
	}
}}
