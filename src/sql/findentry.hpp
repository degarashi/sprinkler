#pragma once
#include "query.hpp"
#include "getvalue.hpp"
#include "skipone.hpp"
#include <QVariant>
#include <QStringBuilder>

namespace dg::sql {
	namespace detail {
		template <class T>
		const char* NullValCheck(const T& val) {
			Q_UNUSED(val)
			return " =?";
		}
		inline const char* NullValCheck(const QVariant& val) {
			if(val.isValid() && !val.isNull())
				return "=?";
			return " IS NULL";
		}
		template <class T>
		const char* NullValCheck(const std::optional<T>& val) {
			if(val)
				return "=?";
			return " IS NULL";
		}
	}
	inline QString FindEntryQuery(const QString& str, std::nullptr_t) {
		return str;
	}
	template <class Str, class Val, class... Args>
	QString FindEntryQuery(Str str, std::nullptr_t, const char* key, const Val& value, const Args&... args) {
		return FindEntryQuery(str % " AND " % key % detail::NullValCheck(value), nullptr, args...);
	}
	template <class Val, class... Args>
	QString FindEntryQuery(const char* key, const Val& value, const Args&... args) {
		return FindEntryQuery(QString("WHERE ") % key % detail::NullValCheck(value), nullptr, args...);
	}
	template <class... Args>
	auto FindEntryId(const char* table, const char* target, const Args&... args) {
		std::optional<int64_t> ret;
		detail::SkipOne(
			[&ret, &args..., table, target](const auto&... args1){
				ret = GetValue<int64_t>(
					Query(
						QString("SELECT %1 FROM ").arg(target) % table % " " % FindEntryQuery(args...) % ";",
						args1...
					)
				);
			},
			args...
		);
		return ret;
	}
	template <class... Args>
	int64_t FindEntryIdNum(const char* table, const char* target, const Args&... args) {
		if(const auto ret = FindEntryId(table, target, args...))
			return *ret;
		throw std::runtime_error("id not found");
	}
}
