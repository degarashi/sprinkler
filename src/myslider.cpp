#include "myslider.hpp"
#include "ui_myslider.h"
#include <QAbstractItemModel>

namespace dg {
	MySlider::MySlider(QWidget *parent):
		QWidget(parent),
		_ui(new Ui::MySlider),
		_model(nullptr),
		_index(-1)
	{
		_ui->setupUi(this);
		_ui->lbValue->setFixedWidth(24);
		connect(_ui->slValue, SIGNAL(valueChanged(int)),
				this, SLOT(valueChanged(int)));
	}
	void MySlider::setSuffix(const QString& sfx) {
		_suffix = sfx;
	}
	void MySlider::setName(const QString& name) {
		_ui->lbName->setText(name);
	}
	QSlider* MySlider::refSlider() const noexcept {
		return _ui->slValue;
	}
	void MySlider::setModel(QAbstractItemModel* model, const int index) {
		_model = model;
		_index = index;
		connect(_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
				this, SLOT(dataChanged(QModelIndex,QModelIndex,QVector<int>)));
	}
	void MySlider::dataChanged(const QModelIndex& lt, const QModelIndex& br, const QVector<int>& role) {
		if(lt.row() == _index) {
			const int val = _fromVariant(_model->data(_model->index(_index,0), Qt::EditRole));
			_ui->slValue->setValue(val);
		}
	}
	void MySlider::valueChanged(const int n) {
		_ui->lbValue->setText(_makeValueString(n) + _suffix);
		if(_model)
			_model->setData(_model->index(_index, 0), _toModel(n));
	}
}
