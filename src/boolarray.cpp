#include "boolarray.hpp"
#include <QDebug>
#include <QRect>
#include <cstring>
#include <cassert>

namespace dg {
	BoolArray::BoolArray():
		width(0),
		height(0)
	{}
	BoolArray::BoolArray(const size_t w, const size_t h):
		value(new bool[w*h]),
		width(w),
		height(h)
	{}
	BoolArray::BoolArray(const BoolArray& b):
		value(new bool[b.width * b.height]),
		width(b.width),
		height(b.height)
	{
		std::memcpy(value.get(), b.value.get(), width*height*sizeof(bool));
	}
	BoolArray& BoolArray::operator = (const BoolArray& b) {
		BoolArray tmp(b);
		std::swap(*this, tmp);
		return *this;
	}
	void BoolArray::fillAll(const bool b) {
		std::fill(value.get(), value.get() + width*height, b);
	}
	void BoolArray::fillRect(const QRect r, const bool b) {
		assert(size_t(r.x()) < width &&
				size_t(r.y()) < height &&
				size_t(r.right()) < width &&
				size_t(r.bottom()) < height);

		for(int i=r.y() ; i<=r.bottom() ; i++) {
			bool* row = value.get() + i*width;
			for(int j=r.x() ; j<=r.right() ; j++)
				row[j] = b;
		}
	}
	void BoolArray::print() const {
		qDebug() << "";
		for(size_t i=0 ; i<height ; i++) {
			auto dbg = qDebug();
			auto* row = value.get() + i*width;
			for(size_t j=0 ; j<width ; j++)
				dbg << (row[j] ? '*' : '-');
		}
		qDebug() << "";
	}
}
