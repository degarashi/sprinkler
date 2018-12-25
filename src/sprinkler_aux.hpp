#pragma once
#include "lubee/src/rect.hpp"

namespace dg {
	class CellBoard;
	struct AspSize {
		float			aspect;
		lubee::RectI	rect;
	};
	using AspSizeV = std::vector<AspSize>;
	// アスペクト比とそれに対応する最大サイズ
	AspSizeV CalcAspSize(const CellBoard& qmap, float minAsp, float maxAsp, float diffAsp);
}
