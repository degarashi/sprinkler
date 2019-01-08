#include "query.hpp"
#include "error.hpp"
#include <QSqlQuery>

namespace dg::sql {
	void Query(QSqlQuery& q) {
		if(!q.exec())
			ThrowError(q);
	}
	void Batch(QSqlQuery& q) {
		if(!q.execBatch())
			ThrowError(q);
	}
}
