#include "watchlist.hpp"
#include "ui_watchlist.h"
#include "watcher.hpp"
#include "rectview.hpp"
#include "quantizer.hpp"
#include "qtw_notifier.hpp"

namespace dg {
	WatchList::WatchList(QAbstractItemModel* model, QWidget *parent) :
		GeomRestore("WatchList", parent),
		_ui(new Ui::WatchList)
	{
		_ui->setupUi(this);
		_ui->watchList->addAction(_ui->actionRemoveWatch);
		_ui->watchList->setModel(model);
	}
	QHBoxLayout* WatchList::getAddArea() const {
		return _ui->buttonArea;
	}
	void WatchList::on_actionRemoveWatch_triggered() {
		const QModelIndex index = _ui->watchList->currentIndex();
		if(index.isValid()) {
			QAbstractItemModel* model = _ui->watchList->model();
			model->removeRow(index.row());
		}
	}
}
