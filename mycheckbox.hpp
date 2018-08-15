#pragma once
#include <QCheckBox>

class QAbstractItemModel;
namespace dg {
	class MyCheckbox :
		public QCheckBox
	{
		Q_OBJECT
		private:
			QAbstractItemModel*		_model;
			int						_index;
		public:
			explicit MyCheckbox(QWidget* parent=nullptr);
			void setModel(QAbstractItemModel* m, const int index);
		private slots:
			void valueChanged(bool b);
			void dataChanged(const QModelIndex& lt, const QModelIndex& br, const QVector<int>& role);
	};
}
