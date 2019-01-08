#pragma once
#include "query.hpp"
#include "error.hpp"
#include <QSqlDatabase>
#include <exception>

namespace dg::sql {
	// ネスト対応
	template <class Proc>
	void Transaction(QSqlDatabase db, Proc&& proc) {
		// セーブポイント名を決定するカウンタ
		// (とりあえず現時点ではシングルスレッドを想定)
		static int s_count = 0;
		// 一度Transactionを開始してみてFalseが返るようならネストしていると判断
		const bool b = db.transaction();
		if(!b) {
			// セーブポイントを定義
			sql::Query(QString("SAVEPOINT Nest_%1").arg(s_count++));
		}
		try {
			proc();
			if(b)
				db.commit();
			else
				--s_count;
		} catch(const std::exception& e) {
			if(b)
				db.rollback();
			else {
				// セーブポイントを復元
				Q_ASSERT(s_count > 0);
				sql::Query(QString("ROLLBACK TO Nest_%1").arg(--s_count));
			}
			throw;
		}
	}
}
