#include "error.hpp"
#include <QStringBuilder>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

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
}
