#pragma once
#include "geom_restore.hpp"
#include <memory>

namespace Ui {
	class WatchList;
}
class QAbstractItemModel;
class QHBoxLayout;
namespace dg {
	class WatchList :
		public GeomRestore
	{
		Q_OBJECT
		public:
			explicit WatchList(QAbstractItemModel* model, QWidget* parent=nullptr);
			QHBoxLayout* getAddArea() const;
		signals:
			void onAddWatchRequested();
		public slots:
			void on_actionRemoveWatch_triggered();
		private:
			std::shared_ptr<Ui::WatchList>	_ui;
	};
}
