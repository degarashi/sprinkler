#pragma once
#include "lubee/src/range.hpp"

namespace dg::place {
	//! 画像サンプル収集パラメータ
	struct Param {
		//! 縮小サイズ範囲
		lubee::RangeF	sizeRange;
		//! 面積比
		size_t			nSample;
	};
}
