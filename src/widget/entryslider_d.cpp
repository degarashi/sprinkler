#include "entryslider_d.hpp"
#include <iomanip>
#include <cmath>

namespace dg::widget {
	int EntrySliderD::_toUi(const float v) const {
		return static_cast<int>(std::round(v * 100));
	}
	float EntrySliderD::_fromUi(const int value) const {
		return float(value) / 100;
	}
	QString EntrySliderD::_makeValueString(const float value) const {
		std::stringstream ss;
		ss << std::showpoint << std::setw(2) << value;
		return QString::fromStdString(ss.str());
	}
	void EntrySliderD::_onValueChanged() {
		emit valueChanged(_fromUi(_slider()->value()));
	}
}
