#pragma once
#include <cstdint>
#include <string>

namespace dg {
	struct Version {
		using Num = uint32_t;
		static Num Major() noexcept;
		static Num Minor() noexcept;
		static Num Release() noexcept;
		static const std::string& GetString();
	};
}
