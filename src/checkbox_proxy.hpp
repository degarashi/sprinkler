#pragma once
#include <QSortFilterProxyModel>

namespace dg {
	class ColumnTarget;
	class CheckBoxProxy :
		public QSortFilterProxyModel
	{
		Q_OBJECT
		private:
			using base_t = QSortFilterProxyModel;
			ColumnTarget*		_column;
		public:
			CheckBoxProxy(ColumnTarget* column, QObject* parent=nullptr);
			void sort(int column, Qt::SortOrder order) override;
	};
}
