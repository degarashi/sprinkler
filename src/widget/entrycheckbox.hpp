#pragma once
#include <QCheckBox>

class QAbstractItemModel;
namespace dg::widget {
	class EntryCheckbox :
		public QCheckBox
	{
		Q_OBJECT
		private:
			QAbstractItemModel*		_model;
			int						_index;
		public:
			explicit EntryCheckbox(QWidget* parent=nullptr);
			void setModel(QAbstractItemModel* m, const int index);
	};
}
