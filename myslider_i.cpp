#include "myslider_i.hpp"
#include <QVariant>

namespace dg {
	QString MySliderI::_makeValueString(const int value) const {
		return QString("%1").arg(value);
	}
	int MySliderI::_fromVariant(const QVariant& v) {
		return v.toInt();
	}
	QVariant MySliderI::_toModel(const int value) {
		return QVariant(value);
	}
}
