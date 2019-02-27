#pragma once
#include "lubee/src/fwd.hpp"
#include <QRect>
class QSizeF;
class QPointF;

QPointF operator * (const QPointF& p, const QSizeF& s);
QPoint operator * (const QPoint& p, const QSize& s);
QPoint ToQPoint(const QPointF& p);
QPoint ToQPoint(const lubee::PointI& p);

QSize ToQSize(const QSizeF& s);
QSizeF operator * (const QSizeF s0, const QSizeF s1);
QRect operator * (const QRect& r, const QSize s);
QRectF operator * (const QRectF& r, const QSizeF s);
QRectF& operator *= (QRectF& r, const QSizeF s);
QRectF operator + (const QRectF& r, const QPointF ofs);
QRectF& operator += (QRectF& r, const QPointF s);
QRect ToQRect(const QRectF& r);
