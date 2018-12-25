#pragma once
#include <QSqlTableModel>
#include "checkbox_model.hpp"

namespace dg {
	class CheckBoxSqlModel :
		public CheckBoxModel<QSqlTableModel>
	{
		Q_OBJECT
		private:
			using base_t = CheckBoxModel<QSqlTableModel>;
			QString _makeWhereClause(const QModelIndex& index) const;

		public:
			CheckBoxSqlModel(ColumnTarget* column, QObject* parent=nullptr);

			QVariant data(const QModelIndex &idx, int role=Qt::DisplayRole) const override;
			bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;
	};
}
