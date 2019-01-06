#pragma once
#include "lubee/src/range.hpp"

namespace dg::place {
	//! 画像サンプル収集パラメータ
	struct Param {
		//! 平均画像枚数
		size_t			avgImage;
		//! 面積比
		size_t			nSample;
	};
}
