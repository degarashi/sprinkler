#include "version.hpp"

namespace dg {
	Version::Num Version::Major() noexcept {
		return 0;
	}
	Version::Num Version::Minor() noexcept {
		return 0;
	}
	Version::Num Version::Release() noexcept {
		return 4;
	}
	const std::string& Version::GetString() {
		static std::string ret =
			std::to_string(Major())
			+ "."
			+ std::to_string(Minor())
			+ "."
			+ std::to_string(Release())
		;
		return ret;
	}
}
