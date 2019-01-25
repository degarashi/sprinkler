#pragma once
#include "lubee/src/size.hpp"
#include "../idtype.hpp"

namespace dg::place {
	struct Selected {
		ImageId			id;
		lubee::SizeI	fitSize;

		// fitSizeを量子化
		lubee::SizeI getQuantizedSize(const size_t qs) const noexcept {
			return {
				int((fitSize.width+int(qs)-1)/int(qs)),
				int((fitSize.height+int(qs)-1)/int(qs))
			};
		}
		lubee::SizeI getQuantizeScaledSize(const float r, const size_t qs) const noexcept {
			return Selected{0, {
				int(fitSize.width * r),
				int(fitSize.height * r)
			}}.getQuantizedSize(qs);
		}
	};
	using SelectedV = std::vector<Selected>;
}
