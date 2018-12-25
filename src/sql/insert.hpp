#pragma once
#include "findentry.hpp"

namespace dg::sql {
	int64_t GetMaxId(const char* table);

	namespace detail {
		template <class Str>
		QString ConcatKey(Str result, bool) {
			return result;
		}
		template <class Str, class Val, class... Args>
		QString ConcatKey(Str str, const bool first, const char* key, const Val&, const Args&... args) {
			return ConcatKey(str % ", " % key, false, args...);
		}
		template <class Str, class Val, class... Args>
		QString ConcatKey(Str str, const char* key, const Val&, const Args&... args) {
			return ConcatKey(str % key, false, args...);
		}

		template <class Str>
		QString Placeholder(Str str, std::integral_constant<std::size_t, 0>) {
			return str;
		}
		template <class Str, std::size_t N>
		QString Placeholder(Str str, std::integral_constant<std::size_t, N>) {
			return Placeholder(str % ", ?", std::integral_constant<std::size_t, N-1>());
		}
		template <std::size_t N>
		QString Placeholder() {
			return Placeholder(QString("?"), std::integral_constant<std::size_t, N-1>());
		}
	}
	template <class... Args>
	void InsertInto(const char* table, const Args&... args) {
		Q_ASSERT((sizeof...(args) & 1) == 0);
		detail::SkipOne(
			[table, &args...](const auto&... args1){
				QueryWithNull(
					QString("INSERT INTO ") % table % "("
					% detail::ConcatKey(QString(""), args...) % ")\n"
					% "VALUES(" % detail::Placeholder<sizeof...(args)/2>() % ");",
					args1...
				);
			},
			args...
		);
	}
	template <class... Args>
	bool InsertIntoIfNotExist(const char* table, const Args&... args) {
		QSqlQuery q;
		q.prepare(
			QString("INSERT INTO ") % table % "(" % detail::ConcatKey(QString(""), args...) % ")"
			% "SELECT " % detail::Placeholder<sizeof...(args)/2>() % "\n"
			% "WHERE NOT EXISTS (SELECT 1 FROM " % table % " " % FindEntryQuery(args...) % ");"
		);
		detail::SkipOne(
			[&q](const auto&... args){
				detail::AddBind_WithNull(q, args...);
				detail::AddBind(q, args...);
			},
			args...
		);
		Query(q);
		return q.numRowsAffected() == 1;
	}
	template <class... Args>
	std::pair<int64_t, bool> InsertIntoIfNotExist_GetId(const char* table, const Args&... args) {
		if(!InsertIntoIfNotExist(table, args...)) {
			const auto num = FindEntryId(table, "id", args...);
			Q_ASSERT(num);
			return {*num, false};
		}
		return {GetMaxId(table), true};
	}
	template <class... Args>
	int64_t InsertInto_GetId(const char* table, const Args&... args) {
		InsertInto(table, args...);
		return GetMaxId(table);
	}
}
