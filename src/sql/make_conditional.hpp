#pragma once
#include <QVariant>
#include <QStringBuilder>

namespace dg::sql {
	namespace detail {
		// 引数がQVariantやoptionalにおけるNull値であれば IS NULLを、それ以外は=?を返す
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

		inline QString MakeConditionalExpression(const QString& str, std::nullptr_t) {
			return str;
		}
		template <class Str, class Val, class... Args>
		QString MakeConditionalExpression(Str str, std::nullptr_t, const char* key, const Val& value, const Args&... args) {
			return MakeConditionalExpression(str % " AND " % key % NullValCheck(value), nullptr, args...);
		}
	}
	template <class Val, class... Args>
	QString MakeConditionalExpression(const char* key, const Val& value, const Args&... args) {
		return detail::MakeConditionalExpression(QString(key) % detail::NullValCheck(value), nullptr, args...);
	}
}
