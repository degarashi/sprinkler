#include "entryslider_i.hpp"
#include <QVariant>

namespace dg::widget {
	int EntrySliderI::_toUi(const int v) const {
		return v;
	}
	int EntrySliderI::_fromUi(const int v) const {
		return v;
	}
	QString EntrySliderI::_makeValueString(const int value) const {
		return QString("%1").arg(value);
	}
	void EntrySliderI::_onValueChanged() {
		emit valueChanged(_slider()->value());
	}
}
