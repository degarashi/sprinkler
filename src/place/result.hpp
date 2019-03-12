#pragma once
#include "lubee/src/rect.hpp"
#include "../idtype.hpp"
#include <vector>

namespace dg::place {
	struct Result {
		ImageId			id;
		lubee::RectI	rect;	// 画像を配置する位置(Qs単位)
	};
	using ResultV = std::vector<Result>;
}
