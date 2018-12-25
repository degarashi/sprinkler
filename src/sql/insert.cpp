#include "insert.hpp"

namespace dg::sql {
	int64_t GetMaxId(const char* table) {
		QSqlQuery q;
		q.prepare(QStringLiteral("SELECT MAX(id) FROM ") % table);
		Query(q);
		if(const auto id = GetValue<int64_t>(q))
			return *id;
		return 0;
	}
}
