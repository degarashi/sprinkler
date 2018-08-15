#include "myspinbox.hpp"
#include <QAbstractItemModel>

namespace dg {
	MySpinbox::MySpinbox(QWidget* parent):
		QSpinBox(parent),
		_index(-1),
		_model(nullptr)
	{
		connect(this, SIGNAL(valueChanged(int)),
				this, SLOT(onValueChanged(int)));
	}
	void MySpinbox::setModel(QAbstractItemModel* m, const int index) {
		_model = m;
		_index = index;
	}
	void MySpinbox::onValueChanged(const int n) {
		if(_model) {
			_model->setData(_model->index(_index, 0), n);
		}
	}
}
