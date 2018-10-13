#include "myslider_d.hpp"
#include <iomanip>
#include <cmath>

namespace dg {
	int MySliderD::ToUi(const float v) {
		return static_cast<int>(std::round(v * 100));
	}
	float MySliderD::FromUi(const int value) {
		return float(value) / 100;
	}
	QString MySliderD::_makeValueString(const int value) const {
		std::stringstream ss;
		ss << std::showpoint << std::setw(2) << FromUi(value);
		return QString::fromStdString(ss.str());
	}
	void MySliderD::_onValueChanged() {
		emit valueChanged(FromUi(_slider()->value()));
	}
	void MySliderD::setRange(const lubee::RangeF r) {
		_slider()->setRange(ToUi(r.from), ToUi(r.to));
	}
	void MySliderD::setValue(const float value) {
		_slider()->setValue(ToUi(value));
	}
}
