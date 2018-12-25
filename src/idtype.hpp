#pragma once
#include <optional>
#include <vector>

namespace dg {
	using PrimaryKey = int32_t;
	using TagId = PrimaryKey;
	using TagIdV = std::vector<TagId>;
	using TagIdOpt = std::optional<TagId>;
	using ImageId = PrimaryKey;
	using ImageIdV = std::vector<ImageId>;
	using DirId = PrimaryKey;
	using DirIdV = std::vector<DirId>;
	using DirIdOpt = std::optional<DirId>;
}
