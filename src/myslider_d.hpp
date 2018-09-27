#pragma once
#include "myslider.hpp"

namespace dg {
	class MySliderD :
		public MySlider
	{
		Q_OBJECT

		public:
			using MySlider::MySlider;
		protected:
			QString _makeValueString(int value) const override;
			int _fromVariant(const QVariant& v) override;
			QVariant _toModel(int value) override;
	};
}
