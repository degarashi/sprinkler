#include "myslider_d.hpp"
#include <QVariant>
#include <iomanip>

namespace dg {
	QString MySliderD::_makeValueString(const int value) const {
		std::stringstream ss;
		ss << std::showpoint << std::setw(2) << float(value) / 100;
		return QString::fromStdString(ss.str());
	}
	int MySliderD::_fromVariant(const QVariant& v) {
		return int(v.toFloat() * 100);
	}
	QVariant MySliderD::_toModel(const int value) {
		return QVariant(float(value) / 100);
	}
}
