#include "entrycheckbox.hpp"
#include <QAbstractItemModel>

namespace dg::widget {
	EntryCheckbox::EntryCheckbox(QWidget* parent):
		QCheckBox(parent),
		_model(nullptr),
		_index(-1)
	{
		connect(this, &EntryCheckbox::toggled,
			this, [this](const bool b){
				if(_model) {
					_model->setData(_model->index(_index, 0), b);
				}
			});
	}
	void EntryCheckbox::setModel(QAbstractItemModel* m, const int index) {
		_model = m;
		_index = index;
		connect(_model, &QAbstractListModel::dataChanged,
			this, [this](const QModelIndex& lt, const QModelIndex& br, const QVector<int>& role){
				if(lt.row() == _index)
					setChecked(_model->data(_model->index(_index,0), Qt::EditRole).toBool());
			});
	}
}
