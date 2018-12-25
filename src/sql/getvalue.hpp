#pragma once
#include "value.hpp"
#include <exception>
#include <vector>

namespace dg::sql {
	template <class T, class Q>
	std::optional<T> GetValue(Q&& q, const size_t index=0, const bool advance=true) {
		if(!advance || q.next()) {
			const QVariant val = q.value(index);
			if(!val.isNull())
				return ConvertQV<T>(val);
		}
		return std::nullopt;
	}
	template <class T, class Q>
	T GetRequiredValue(Q&& q, const size_t index=0, const bool advance=true) {
		if(const auto ret = GetValue<T>(q, index, advance))
			return *ret;
		throw std::runtime_error("value not found");
	}
	template <class T, class Q>
	std::vector<T> GetValues(Q&& q, const size_t index=0) {
		std::vector<T> ret;
		while(q.next()) {
			const QVariant val = q.value(index);
			if(val.isNull())
				throw std::runtime_error("null value detected");
			ret.emplace_back(ConvertQV<T>(val));
		}
		return ret;
	}
}
