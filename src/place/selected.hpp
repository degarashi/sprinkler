#pragma once
#include "lubee/src/size.hpp"
#include "../idtype.hpp"

namespace dg::place {
	struct Selected {
		ImageId			id;
		lubee::SizeI	modifiedSize,
						quantizedSize;
		bool			important;
	};
	using SelectedV = std::vector<Selected>;
}
