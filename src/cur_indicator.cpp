#include "cur_indicator.hpp"

namespace dg {
	void CurIndicator::_makeText() {
		setText(QString("%1 / %2").arg(_cur).arg(_num));
	}
	void CurIndicator::onSprinkleCounterChanged(const size_t shown, const size_t notshown) {
		_cur = shown;
		_num = shown + notshown;
		_makeText();
		update();
	}
}
