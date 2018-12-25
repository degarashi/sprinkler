#include "colorframe.hpp"
#include <QPainter>

namespace dg::widget {
	ColorFrame::ColorFrame(QWidget* parent):
		QWidget(parent),
		_lineWidth(3)
	{
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_TranslucentBackground);
		setAttribute(Qt::WA_TransparentForMouseEvents);
	}
	void ColorFrame::setWidth(const int w) {
		_lineWidth = w;
	}
	void ColorFrame::paintEvent(QPaintEvent* e) {
		QPainter pt(this);
		const QSize s = size();
		QPen pen;
		pen.setCapStyle(Qt::FlatCap);
		pen.setJoinStyle(Qt::MiterJoin);
		pen.setWidth(_lineWidth);
		pen.setColor(Qt::green);
		pt.setPen(pen);
		pt.drawRect(1, 1, s.width()-_lineWidth, s.height()-_lineWidth);
	}
}
