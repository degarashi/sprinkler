#pragma once
#include "entryslider.hpp"

namespace dg::widget {
	class EntrySliderD :
		public EntrySlider<float>
	{
		Q_OBJECT
		protected:
			int _toUi(float v) const override;
			float _fromUi(int v) const override;
			QString _makeValueString(float value) const override;
			void _onValueChanged() override;
		signals:
			void valueChanged(float v);
		public:
			using EntrySlider::EntrySlider;
	};
}
