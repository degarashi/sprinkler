#pragma once
#include "widget/gr_widget.hpp"
#include <memory>

namespace Ui {
	class WatchList;
}
class QAbstractItemModel;
class QHBoxLayout;
namespace dg {
	class WatchList :
		public widget::GeomRestore_Widget
	{
		Q_OBJECT
		private:
			using UI_S = std::shared_ptr<Ui::WatchList>;
		private:
			UI_S	_ui;

		public:
			explicit WatchList(QAbstractItemModel* model, QWidget* parent=nullptr);
			QHBoxLayout* getAddArea() const;
		signals:
			void requestAddWatch();
		public slots:
			void on_actionRemoveWatch_triggered();
	};
}
