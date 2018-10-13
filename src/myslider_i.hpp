#pragma once
#include "myslider.hpp"
#include "lubee/range.hpp"

namespace dg {
	class MySliderI :
		public MySlider
	{
		Q_OBJECT
		protected:
			QString _makeValueString(int value) const override;
			void _onValueChanged() override;
		signals:
			void valueChanged(int v);
		public:
			using MySlider::MySlider;
			void setRange(lubee::RangeI r);
			void setValue(int value);
	};
}
