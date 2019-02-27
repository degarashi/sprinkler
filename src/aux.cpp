#include "aux.hpp"
#include "q_rs_op.hpp"
#include <cmath>

float GetRatio(const QSizeF orig, const QSizeF scr) {
	const auto	asp0 = orig.width() / orig.height(),
				asp1 = scr.width() / scr.height();
	if(asp0 >= asp1) {
		return float(scr.width() / orig.width());
	} else {
		return float(scr.height() / orig.height());
	}
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
	return ToQRect(r * sc + ofs);
}
QSize AspectKeepScale(const QSize target, const QSize size) {
	const auto ar = [](const QSize s){
		return float(s.width()) / s.height();
	};
	const float asp0 = ar(target),
				asp1 = ar(size);
	if(asp0 > asp1) {
		// 縦を補正
		return QSize{
			int(std::floor(size.width() * (float(target.height()) / size.height()) + .5f)),
			target.height()
		};
	} else {
		// 横を補正
		return QSize{
			target.width(),
			int(std::floor(size.height() * (float(target.width()) / size.width()) + .5f)),
		};
	}
}
