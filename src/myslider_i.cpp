#include "myslider_i.hpp"
#include <QVariant>

namespace dg {
	QString MySliderI::_makeValueString(const int value) const {
		return QString("%1").arg(value);
	}
	void MySliderI::setRange(const lubee::RangeI r) {
		_slider()->setRange(r.from, r.to);
	}
	void MySliderI::_onValueChanged() {
		emit valueChanged(_slider()->value());
	}
	void MySliderI::setValue(const int value) {
		_slider()->setValue(value);
	}
}
