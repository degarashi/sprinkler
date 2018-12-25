#include "checkbox_sqlmodel.hpp"
#include "column_target.hpp"
#include "sql/getvalue.hpp"
#include "sql/query.hpp"
#include <QStringBuilder>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlDriver>

namespace dg {
	CheckBoxSqlModel::CheckBoxSqlModel(ColumnTarget* column, QObject* parent):
		base_t(column, parent)
	{}
	QString CheckBoxSqlModel::_makeWhereClause(const QModelIndex& index) const {
		// プライマリキーが1つ以上ないと更新不可
		const QSqlIndex idx = primaryKey();
		Q_ASSERT(idx.count() > 0);
		QSqlRecord rec;
		for(int i=0 ; i<idx.count() ; i++) {
			QSqlField f = idx.field(i);
			const int keyColumn = record().indexOf(f.name());
			f.setValue(data(this->index(index.row(), keyColumn), Qt::DisplayRole));
			rec.append(f);
		}
		const QString whereSt = QSqlDatabase::database().driver()->sqlStatement(
			QSqlDriver::WhereStatement,
			tableName(),
			rec,
			false
		);
		Q_ASSERT(!whereSt.isEmpty());
		return whereSt;
	}
	bool CheckBoxSqlModel::setData(const QModelIndex &index, const QVariant &value, const int role) {
		// 該当のカラム番号 & CheckStateRoleの時だけ処理を変える
		if(_isCheckBox(index, role)) {
			QSqlRecord rec;
			rec.append(
				QSqlField(
					record().fieldName(index.column())
				)
			);
			rec.field(0).setValue(
				static_cast<bool>(value == Qt::Checked)
			);
			const QString updateSt = QSqlDatabase::database().driver()->sqlStatement(
				QSqlDriver::UpdateStatement,
				tableName(),
				rec,
				false
			);
			Q_ASSERT(!updateSt.isEmpty());

			const auto q = sql::Query(updateSt % ' ' % _makeWhereClause(index));
			Q_ASSERT(q.numRowsAffected() == 1);
			return true;
		}
		// それ以外はベースクラスの挙動
		return base_t::setData(index, value, role);
	}
	QVariant CheckBoxSqlModel::data(const QModelIndex &idx, const int role) const {
		if(_isCheckBox(idx, role)) {
			const QString cName = record().fieldName(idx.column());
			const auto value = sql::GetRequiredValue<int>(
				sql::Query(
					"SELECT " % cName % " FROM " % tableName() % ' ' % _makeWhereClause(idx)
				)
			);
			return static_cast<bool>(value)
				? Qt::Checked : Qt::Unchecked;
		}
		// それ以外はデフォルトの挙動
		return base_t::data(idx, role);
	}
}
