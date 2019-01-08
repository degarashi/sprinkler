#pragma once

class QSqlQuery;
class QSqlDatabase;
class QString;
namespace dg::sql {
	void ThrowError[[noreturn]](QSqlDatabase& db, const QString& message);
	void ThrowError[[noreturn]](QSqlQuery& q);
}
