#pragma once
#include "myslider.hpp"
#include "lubee/range.hpp"

namespace dg {
	class MySliderD :
		public MySlider
	{
		Q_OBJECT
		private:
			static float FromUi(int value);
			static int ToUi(float value);
		protected:
			QString _makeValueString(int value) const override;
			void _onValueChanged() override;
		signals:
			void valueChanged(float v);
		public:
			using MySlider::MySlider;
			void setRange(lubee::RangeF r);
			void setValue(float value);
	};
}
