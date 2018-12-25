#include "entryspinbox.hpp"
#include <QAbstractItemModel>

namespace dg::widget {
	EntrySpinbox::EntrySpinbox(QWidget* parent):
		QSpinBox(parent),
		_index(-1),
		_model(nullptr)
	{
		connect(this, qOverload<int>(&QSpinBox::valueChanged),
			this, [this](const int n){
				if(_model) {
					_model->setData(_model->index(_index, 0), n);
				}
			});
	}
	void EntrySpinbox::setModel(QAbstractItemModel* m, const int index) {
		_model = m;
		_index = index;
	}
}
