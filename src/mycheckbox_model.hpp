#pragma once
#include <QStandardItemModel>
#include "checkbox_model.hpp"

namespace dg {
	class MyCheckBoxModel :
		public CheckBoxModel<QStandardItemModel>
	{
		Q_OBJECT
		private:
			using base_t = CheckBoxModel<QStandardItemModel>;
		public:
			using base_t::base_t;
	};
}
