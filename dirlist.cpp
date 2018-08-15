#include "dirlist.hpp"
#include "ui_dirlist.h"
#include <QFileDialog>

namespace dg {
	DirList::DirList(QAbstractItemModel* model, QWidget* parent) :
		GeomRestore("DirList", parent),
		_ui(new Ui::DirList)
	{
		_ui->setupUi(this);
		_ui->table->setModel(model);
		_ui->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	}
	void DirList::addDir() {
		QString path(QFileDialog::getExistingDirectory(this, "Select Image Directory", nullptr));
		if(!path.isEmpty()) {
			QAbstractItemModel* m = _ui->table->model();
			// 既に登録されていたら何もしない
			if(m->match(m->index(0,0), Qt::EditRole, path).empty()) {
				m->insertRow(0);
				m->setData(m->index(0,0), path, Qt::EditRole);
			}
		}
	}
	void DirList::remDir() {
		// とりあえず単体選択時のみ対応
		const QModelIndexList sl = _ui->table->selectionModel()->selectedRows();
		if(sl.size() == 1) {
			QAbstractItemModel* m = _ui->table->model();
			m->removeRow(sl[0].row());
		}
	}
}
