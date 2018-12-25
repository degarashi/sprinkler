#include "rectview.hpp"
#include "aux.hpp"
#include "q_rs_op.hpp"
#include <QPainter>
#include <cmath>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QCheckBox>

namespace dg {
	void RectView::GuiObj::draw(QPainter& p, const QRect& r, const QString& t) const {
		p.setPen(frame);
		p.setBrush(bkg);
		p.drawRect(r);
		p.setPen(text);
		p.drawText(r, t);
	}
	void RectView::GuiObj::fontsize(QPainter& p) const {
		QFont font = p.font();
		font.setPointSize(textSize);
		p.setFont(font);
	}

	RectView::RectView(QWidget *const parent):
		QWidget(parent),
		_bg(Qt::gray),
		_drawQuantized(false),
		_qmap({1,1}),
		_qsize(1)
	{
		_maxrect.setColor(QColor(0,0,255, 200));
		_maxrect.setStyle(Qt::SolidPattern);
		{
			auto& scr = _guiobj.screen;
			scr.bkg = QBrush(Qt::white);
			scr.text = QPen(Qt::black);
			scr.frame = QPen(Qt::black);
			scr.textSize = 16;
		}
		{
			auto& qt = _guiobj.qt;
			qt.bkg = QBrush(QColor(255,0,0,200));
			qt.text = QPen(Qt::white);
			qt.frame = QPen(Qt::black);
			qt.textSize = 16;
		}
		{
			auto& oth = _guiobj.other;
			oth.bkg = QBrush(QColor(128,128,255,200));
			oth.text = QPen(Qt::white);
			oth.frame = QPen(Qt::black);
			oth.textSize = 16;
		}
	}
	void RectView::setMode(const bool q) {
		_drawQuantized = q;
		update();
	}
	void RectView::onGridChanged(const DomainSet& ds, const dg::CellBoard& qm, const size_t qs) {
		_dset = ds;
		_qmap = qm;
		_qsize = qs;
		update();
	}
	void RectView::paintEvent(QPaintEvent* e) {
		// Widgetに表示できる面積(スクリーンとのアスペクト比維持)を計算
		// 仮想デスクトップのサイズ
		QSizeF vscr(_dset.vscr.size());
		// Widgetが最大限使える領域
		const QSize w_area = size();
		const float s = GetRatio(vscr, w_area);
		const QSizeF scale(s, s);
		vscr = vscr * scale;
		const QPoint offset(
			w_area.width()/2 - std::floor(vscr.width()/2),
			w_area.height()/2 - std::floor(vscr.height()/2)
		);

		QPainter pt(this);
		pt.fillRect(e->rect(), Qt::gray);
		if(!_drawQuantized)
			_paintRects(pt, scale, offset);
		else
			_paintRectQuantized(pt, scale, offset);
	}
	void RectView::_paintRects(QPainter& p, const QSizeF sc, const QPoint ofs) const {
		QRegion reg;
		{
			auto& go = _guiobj.screen;
			go.fontsize(p);
			for(auto& r : _dset.domain.screen) {
				const QRect r2 = RectScOfs(r.rect, sc, ofs).marginsRemoved(QMargins(1,1,0,0));
				reg += r2;
				go.draw(p, r2, r.name);
			}
		}
		p.setClipRegion(reg);
		p.setClipping(true);
		{
			auto& go = _guiobj.qt;
			go.fontsize(p);
			for(auto& r : _dset.domain.qt) {
				const QRect r2 = RectScOfs(r.rect, sc, ofs).marginsRemoved(QMargins(1,1,0,0));
				go.draw(p, r2, r.name);
			}
		}
		{
			auto& go = _guiobj.other;
			go.fontsize(p);
			for(auto& r : _dset.domain.other) {
				const QRect r2 = RectScOfs(r.rect, sc, ofs).marginsRemoved(QMargins(1,1,0,0));
				go.draw(p, r2, r.name);
			}
		}
	}
	void RectView::_paintRectQuantized(QPainter& p, const QSizeF sc, const QPoint ofs) const {
		const auto& nb = _qmap.nboard();
		const auto qs = nb.getSize();
		const int QS = _qsize;
		p.fillRect(QRect{ofs, QSize(QS*qs.width*sc.width(), QS*qs.height*sc.height())}, Qt::white);
		QRegion reg;
		for(size_t i=0 ; i<qs.height ; i++) {
			for(size_t j=0 ; j<qs.width ; j++) {
				if(nb.cellAt(j, i).used)
					reg += QRect(j,i,1,1);
			}
		}
		for(auto& r : reg)
			p.fillRect(RectScOfs(r * QSize{QS, QS}, sc, ofs), Qt::darkGray);

		QPen pen;
		pen.setColor(Qt::black);
		pen.setStyle(Qt::DashLine);
		p.setPen(pen);

		const int ox = ofs.x(),
					oy = ofs.y();
		for(size_t i=0 ; i<=qs.height ; i++) {
			const int y = i*QS*sc.height()+oy;
			p.drawLine(
				QPoint(ox, y),
				QPoint(qs.width*QS*sc.width()+ox, y)
			);
		}
		for(size_t j=0 ; j<=qs.width ; j++) {
			const int x = j*QS*sc.width()+ox;
			p.drawLine(
				QPoint(x, oy),
				QPoint(x, qs.height * QS * sc.height() + oy)
			);
		}
	}
}
