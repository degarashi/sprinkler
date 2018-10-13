#include "myslider.hpp"
#include "ui_myslider.h"

namespace dg {
	QSlider* MySlider::_slider() const {
		return _ui->slValue;
	}
	MySlider::MySlider(QWidget *parent):
		QWidget(parent),
		_ui(new Ui::MySlider)
	{
		_ui->setupUi(this);
		_ui->lbValue->setFixedWidth(24);
		connect(
			_ui->slValue,
			&QSlider::valueChanged,
			this,
			[this](const int v){
				_ui->lbValue->setText(_makeValueString(v) + _suffix);
				_onValueChanged();
			}
		);
	}
	void MySlider::setName(const QString& name) {
		_ui->lbName->setText(name);
	}
	void MySlider::setSuffix(const QString& sfx) {
		_suffix = sfx;
	}
}
