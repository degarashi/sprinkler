#pragma once
#include "query.hpp"
#include "getvalue.hpp"
#include "skipone.hpp"
#include "make_conditional.hpp"

namespace dg::sql {
	//! 指定のテーブルから引数の組み合わせに合致するレコードを探し、そのIdを得る
	template <class... Args>
	auto GetIdFromRecord(const char* table, const char* idColumn, const Args&... args) {
		std::optional<int64_t> ret;
		detail::SkipOne(
			[&ret, &args..., table, idColumn](const auto&... args1){
				ret = GetValue<int64_t>(
					Query(
						QString("SELECT %1 FROM ").arg(idColumn) % table % " WHERE " % MakeConditionalExpression(args...) % ";",
						args1...
					)
				);
			},
			args...
		);
		return ret;
	}
	//! GetIdFromRecordを呼び出し、戻り値が無効なら例外を投げる
	template <class... Args>
	int64_t GetIdFromRecordNum(const char* table, const char* idColumn, const Args&... args) {
		if(const auto ret = GetIdFromRecord(table, idColumn, args...))
			return *ret;
		throw std::runtime_error("id not found");
	}
}
