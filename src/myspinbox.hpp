#pragma once
#include <QSpinBox>

class QAbstractItemModel;
namespace dg {
	class MySpinbox :
		public QSpinBox
	{
		Q_OBJECT
		private:
			int						_index;
			QAbstractItemModel*		_model;
		public:
			explicit MySpinbox(QWidget* parent=nullptr);
			void setModel(QAbstractItemModel* m, const int index);
		public slots:
			void onValueChanged(int n);
	};
}
