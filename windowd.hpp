#pragma once
#include <cstring>

namespace dg {
	struct WindowD {
		char value[8];

		WindowD() = default;
		template <class T>
		WindowD(const T t) noexcept: value{} {
			static_assert(sizeof(T) <= sizeof(value));
			std::memcpy(value, &t, sizeof(t));
		}
		template <class T>
		T as() const noexcept {
			return *reinterpret_cast<const T*>(value);
		}
		bool operator == (const WindowD& w) const noexcept {
			return std::memcmp(value, w.value, sizeof(value)) == 0;
		}
	};
}
