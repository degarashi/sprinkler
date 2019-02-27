#pragma once
#include "lubee/src/rect.hpp"
#include "../idtype.hpp"
#include <QVector>

namespace dg::place {
	struct Result {
		ImageId			id;
		lubee::RectI	rect;	// 画像を配置する位置(ピクセル単位)
	};
	using ResultV = QVector<Result>;
}
