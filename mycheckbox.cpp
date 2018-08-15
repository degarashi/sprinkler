#include "mycheckbox.hpp"
#include <QAbstractItemModel>

namespace dg {
	MyCheckbox::MyCheckbox(QWidget* parent):
		QCheckBox(parent),
		_model(nullptr),
		_index(-1)
	{
		connect(this, SIGNAL(toggled(bool)),
				this, SLOT(valueChanged(bool)));
	}
	void MyCheckbox::setModel(QAbstractItemModel* m, const int index) {
		_model = m;
		_index = index;
		connect(_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
				this, SLOT(dataChanged(QModelIndex,QModelIndex,QVector<int>)));
	}
	void MyCheckbox::dataChanged(const QModelIndex& lt, const QModelIndex& br, const QVector<int>& role) {
		if(lt.row() == _index)
			setChecked(_model->data(_model->index(_index,0), Qt::EditRole).toBool());
	}
	void MyCheckbox::valueChanged(const bool b) {
		if(_model) {
			_model->setData(_model->index(_index, 0), b);
		}
	}
}
