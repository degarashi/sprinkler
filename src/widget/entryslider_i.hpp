#pragma once
#include "entryslider.hpp"

namespace dg::widget {
	class EntrySliderI :
		public EntrySlider<int>
	{
		Q_OBJECT
		protected:
			int _toUi(int v) const override;
			int _fromUi(int v) const override;
			QString _makeValueString(int value) const override;
			void _onValueChanged() override;
		signals:
			void valueChanged(int v);
		public:
			using EntrySlider::EntrySlider;
	};
}
