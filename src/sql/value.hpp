#pragma once
#include "lubee/src/meta/enable_if.hpp"
#include <QVariant>

namespace dg::sql {
	namespace detail {
		bool _ConvertQV(const QVariant& v, bool*);
		float _ConvertQV(const QVariant& v, float*);
		QByteArray _ConvertQV(const QVariant& v, QByteArray*);
		QString _ConvertQV(const QVariant& v, QString*);
		template <
			class T,
			ENABLE_IF(sizeof(T)>=4 && std::is_integral_v<T>)
		>
		T _ConvertQV(const QVariant& v, T*) {
			constexpr bool sign = std::is_signed_v<T>;
			if constexpr (sizeof(T) == 8) {
				if constexpr (sign)
					return static_cast<T>(v.toLongLong());
				else
					return static_cast<T>(v.toULongLong());
			} else {
				if constexpr (sign)
					return static_cast<T>(v.toInt());
				else
					return static_cast<T>(v.toUInt());
			}
		}
	}
	template <class T>
	T ConvertQV(const QVariant& v) {
		Q_ASSERT(v.canConvert<T>());
		return detail::_ConvertQV(v, static_cast<T*>(nullptr));
	}
}
