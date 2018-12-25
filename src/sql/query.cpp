#include "query.hpp"
#include <QSqlError>
#include <QSqlQuery>
#include <QStringBuilder>

namespace dg::sql {
	void ThrowError(QSqlDatabase& db, const QString& message) {
		throw std::runtime_error(
			QString(message % '\n' % db.lastError().text()).toStdString()
		);
	}
	void ThrowError(QSqlQuery& q) {
		const QString errMsg(q.lastQuery() % "\n" % q.lastError().text());
		throw std::runtime_error(errMsg.toStdString());
	}
	void Query(QSqlQuery& q) {
		if(!q.exec())
			ThrowError(q);
	}
	void Batch(QSqlQuery& q) {
		if(!q.execBatch())
			ThrowError(q);
	}
}
