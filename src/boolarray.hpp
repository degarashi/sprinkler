#pragma once
#include <memory>

class QRect;
namespace dg {
	struct BoolArray {
		using Bool_U = std::unique_ptr<bool>;
		Bool_U	value;
		size_t	width,
				height;

		BoolArray();
		BoolArray(const BoolArray& ba);
		BoolArray(BoolArray&&) = default;
		BoolArray(const size_t w, const size_t h);
		BoolArray& operator = (const BoolArray&);
		BoolArray& operator = (BoolArray&&) = default;
		void fillAll(const bool b);
		void fillRect(const QRect r, const bool b);
		void print() const;
	};
}
