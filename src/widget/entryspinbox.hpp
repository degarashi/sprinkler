#pragma once
#include <QSpinBox>

class QAbstractItemModel;
namespace dg::widget {
	class EntrySpinbox :
		public QSpinBox
	{
		Q_OBJECT
		private:
			int						_index;
			QAbstractItemModel*		_model;
		public:
			explicit EntrySpinbox(QWidget* parent=nullptr);
			void setModel(QAbstractItemModel* m, const int index);
	};
}
