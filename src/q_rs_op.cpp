#include "q_rs_op.hpp"
#include <cmath>
#include "lubee/src/point.hpp"
#include "lubee/src/size.hpp"

QPointF operator * (const QPointF& p, const QSizeF& s) {
	return QPointF {
		p.x() * s.width(),
		p.y() * s.height()
	};
}
QPoint operator * (const QPoint& p, const QSize& s) {
	return QPoint {
		p.x() * s.width(),
		p.y() * s.height()
	};
}
QPoint ToQPoint(const QPointF& p) {
	return QPoint {
		static_cast<int>(p.x()),
		static_cast<int>(p.y())
	};
}
QPoint ToQPoint(const lubee::PointI& p) {
	return {
		p.x,
		p.y
	};
}

QSize ToQSize(const QSizeF& s) {
	return QSize {
		static_cast<int>(s.width()),
		static_cast<int>(s.height())
	};
}
QSize ToQSize(const lubee::SizeI& s) {
	return {
		s.width,
		s.height
	};
}
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
QRect ToQRect(const QRectF& r) {
	return {
		int(std::floor(r.x())),
		int(std::floor(r.y())),
		int(std::floor(r.right()) - std::floor(r.x())),
		int(std::floor(r.bottom() - std::floor(r.y())))
	};
}
