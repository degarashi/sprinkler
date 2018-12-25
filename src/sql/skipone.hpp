#pragma once
#include <tuple>

namespace dg::sql::detail {
	template <class CB, size_t... Idx, class Tup>
	void _ExpandTuple(const CB& cb, const Tup& tup, std::index_sequence<Idx...>) {
		cb(std::get<Idx>(tup)...);
	}
	template <class CB, class Tup>
	void ExpandTuple(const CB& cb, const Tup& tup) {
		_ExpandTuple(cb, tup, std::make_index_sequence<std::tuple_size_v<Tup>>{});
	}

	template <class CB, class Tup>
	void _SkipOne(const CB& cb, const Tup& tup) {
		ExpandTuple(cb, tup);
	}
	template <class CB, class Tup, class Skip, class Value, class... Args>
	void _SkipOne(const CB& cb, const Tup& tup, const Skip&, const Value& value, const Args&... args) {
		_SkipOne(cb, std::tuple_cat(tup, std::tuple<const Value&>(value)), args...);
	}
	template <class CB, class... Args>
	void SkipOne(const CB& cb, const Args&... args) {
		_SkipOne(cb, std::tuple<>{}, args...);
	}
}
