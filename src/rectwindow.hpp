#pragma once
#include "widget/gr_widget.hpp"
#include <memory>

namespace Ui {
	class RectWindow;
}
namespace dg {
	class CellBoard;
	struct DomainSet;
	class RectWindow :
		public widget::GeomRestore_Widget
	{
		Q_OBJECT
		private:
			std::shared_ptr<Ui::RectWindow>	_ui;
		public slots:
			void onGridChanged(const dg::DomainSet& ds, const dg::CellBoard& qm, size_t qs);
		public:
			explicit RectWindow(QWidget* parent=nullptr);
	};
}
