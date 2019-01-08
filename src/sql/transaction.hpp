#pragma once
#include "query.hpp"
#include "error.hpp"
#include <QSqlDatabase>
#include <exception>

namespace dg::sql {
	template <class Proc>
	void Transaction(QSqlDatabase db, Proc&& proc) {
		if(!db.transaction())
			ThrowError(db, QStringLiteral("can't begin transaction"));
		try {
			proc();
			db.commit();
		} catch(const std::exception& e) {
			db.rollback();
			throw;
		}
	}
}
