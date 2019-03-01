#pragma once
#include "lubee/src/size.hpp"
#include "../idtype.hpp"

namespace dg::place {
	struct Selected {
		ImageId			id;
		// 遺伝子で倍率を1.0とした時のサイズ(= 最大表示ピクセルサイズ)
		lubee::SizeI	size;

		// sizeをqsで量子化(端数は切り上げ)
		lubee::SizeI getQuantizedSize(const size_t qs) const noexcept {
			return {
				int((size.width+int(qs)-1)/int(qs)),
				int((size.height+int(qs)-1)/int(qs))
			};
		}
		// sizeにスケーリング値をかけた物(端数は切り捨て)をqsで量子化
		lubee::SizeI getQuantizeScaledSize(const float r, const size_t qs) const noexcept {
			return Selected{0, {
				int(size.width * r),
				int(size.height * r)
			}}.getQuantizedSize(qs);
		}
	};
	using SelectedV = std::vector<Selected>;
}
