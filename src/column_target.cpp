#include "column_target.hpp"
#include <QSqlDatabase>
#include <QSqlRecord>

namespace dg {
	ColumnTarget::ColumnTarget(QObject* parent):
		QObject(parent)
	{}
	void ColumnTarget::addTarget(const QString& tableName, const char *const *const columnNames, const size_t n) {
		const auto db = QSqlDatabase::database();
		const auto rec = db.record(tableName);
		for(size_t i=0 ; i<n ; i++) {
			const auto idx = rec.indexOf(columnNames[i]);
			Q_ASSERT(idx >= 0);
			addTarget(idx);
		}
	}
	void ColumnTarget::addTarget(const int column) {
		_target.emplace(column);
	}
	void ColumnTarget::remTarget(const int column) {
		if(const auto itr = _target.find(column);
			itr != _target.end())
			_target.erase(itr);
	}
	bool ColumnTarget::has(const int column) const {
		return _target.count(column) == 1;
	}
}
