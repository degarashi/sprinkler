#pragma once
#include <QVariant>
#include <QSqlQuery>

class QSqlDatabase;
class QString;
namespace dg::sql {
	void Query(QSqlQuery& q);
	void Batch(QSqlQuery& q);
	namespace detail {
		inline void AddBind(QSqlQuery&) {}
		template <class Value, class... Args>
		void AddBind(QSqlQuery& q, const Value& value, const Args&... args) {
			const auto v = QVariant::fromValue(value);
			if(!v.isNull())
				q.addBindValue(v);
			AddBind(q, args...);
		}
		inline void AddBind_WithNull(QSqlQuery&) {}
		template <class Value, class... Args>
		void AddBind_WithNull(QSqlQuery& q, const Value& value, const Args&... args) {
			const auto v = QVariant::fromValue(value);
			q.addBindValue(v);
			AddBind_WithNull(q, args...);
		}

		template <bool WithNull, class... Args>
		QSqlQuery Query(std::integral_constant<bool, WithNull>, const QString& str, const Args&... args) {
			QSqlQuery q;
			q.prepare(str);
			if(WithNull)
				AddBind_WithNull(q, args...);
			else
				AddBind(q, args...);
			::dg::sql::Query(q);
			return std::move(q);
		}
	}

	template <class... Args>
	QSqlQuery Query(const QString& str, const Args&... args) {
		return detail::Query(std::false_type(), str, args...);
	}
	template <class... Args>
	QSqlQuery QueryWithNull(const QString& str, const Args&... args) {
		return detail::Query(std::true_type(), str, args...);
	}
}
