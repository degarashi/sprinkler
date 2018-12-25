#pragma once
#include <QRect>

float GetRatio(const QSizeF orig, const QSizeF scr);
QRect QuantifyS(const QRect r, int n);
int Quantify(const int val, int n);
QRect Quantify(const QRect& r, int n);
QRect RectScOfs(const QRect& r, const QSizeF sc, const QPointF ofs);
// アスペクト比を保ったままスケーリング
QSize AspectKeepScale(const QSize target, const QSize size);
