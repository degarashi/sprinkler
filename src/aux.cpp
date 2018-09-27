#include "aux.hpp"
#include <cmath>

QSizeF operator * (const QSizeF s0, const QSizeF s1) {
	return {s0.width() * s1.width(),
			s0.height() * s1.height()};
}
QRect operator * (const QRect& r, const QSize s) {
	const auto w = s.width(),
				h = s.height();
	return {
		r.x() * w,
		r.y() * h,
		r.width() * w,
		r.height() * h
	};
}
QRectF operator * (const QRectF& r, const QSizeF s) {
	const auto w = s.width(),
				h = s.height();
	return {
		r.x() * w,
		r.y() * h,
		r.width() * w,
		r.height() * h
	};
}
QRectF& operator *= (QRectF& r, const QSizeF s) {
	return r = r * s;
}
QRectF operator + (const QRectF& r, const QPointF ofs) {
	return {
		r.x() + ofs.x(),
		r.y() + ofs.y(),
		r.width(),
		r.height()
	};
}
QRectF& operator += (QRectF& r, const QPointF s) {
	return r = r + s;
}
float GetRatio(const QSizeF orig, const QSizeF scr) {
	const float asp0 = orig.width() / orig.height(),
				asp1 = scr.width() / scr.height();
	if(asp0 >= asp1) {
		return scr.width() / orig.width();
	} else {
		return scr.height() / orig.height();
	}
}
QRect ToRect(const QRectF& r) {
	return {
		int(std::floor(r.x())),
		int(std::floor(r.y())),
		int(std::floor(r.right()) - std::floor(r.x())),
		int(std::floor(r.bottom() - std::floor(r.y())))
	};
}
QRect QuantifyS(const QRect r, const int n) {
	return {
		r.x() / n,
		r.y() / n,
		(r.right()+n) / n - (r.x() / n),
		(r.bottom()+n) / n - (r.y() / n)
	};
}
int Quantify(const int val, const int n) {
	return val / n * n;
}
QRect Quantify(const QRect& r, const int n) {
	const auto q = [n](int val){ return Quantify(val, n); };
	return {
		q(r.x()),
		q(r.y()),
		q(r.x() + r.width()) - q(r.x()) + 1,
		q(r.y() + r.height()) - q(r.y()) + 1
	};
}
QRect RectScOfs(const QRect& r, const QSizeF sc, const QPointF ofs) {
	return ToRect(r * sc + ofs);
}
