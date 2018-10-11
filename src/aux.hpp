#include <QRect>
class QSizeF;
class QPointF;

QPointF operator * (const QPointF& p, const QSizeF& s);
QPoint operator * (const QPoint& p, const QSize& s);
QPoint ToQPoint(const QPointF& p);

QSize ToQSize(const QSizeF& s);
QSizeF operator * (const QSizeF s0, const QSizeF s1);
QRect operator * (const QRect& r, const QSize s);
QRectF operator * (const QRectF& r, const QSizeF s);
QRectF& operator *= (QRectF& r, const QSizeF s);
QRectF operator + (const QRectF& r, const QPointF ofs);
QRectF& operator += (QRectF& r, const QPointF s);
float GetRatio(const QSizeF orig, const QSizeF scr);
QRect QuantifyS(const QRect r, int n);
int Quantify(const int val, int n);
QRect Quantify(const QRect& r, int n);
QRect RectScOfs(const QRect& r, const QSizeF sc, const QPointF ofs);
QRect ToRect(const QRectF& r);
// アスペクト比を保ったままスケーリング
QSize AspectKeepScale(const QSize target, const QSize size);
