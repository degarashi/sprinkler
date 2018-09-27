#pragma once
#include "domain.hpp"
#include "histgram/src/cell.hpp"
#include <QWidget>
#include <QPen>

namespace dg {
	class RectView : public QWidget {
		Q_OBJECT
		private:
			struct GuiObj {
				QPen	text,
						frame;
				QBrush	bkg;
				int		textSize;

				void draw(QPainter& p, const QRect& r, const QString& t) const;
				void fontsize(QPainter& p) const;
			} _guiobj[DomainType::_Num];

			DomainSet	_dset;
			CellBoard	_qmap;
			size_t		_qsize;

			QBrush		_bg,
						_maxrect;
			bool		_drawQuantized;

			void _paintRects(QPainter& p, QSizeF scale, QPoint offset) const;
			void _paintRectQuantized(QPainter& p, QSizeF scale, QPoint offset) const;

		public slots:
			void setMode(bool q);
			void onGridChanged(const dg::DomainSet& ds, const dg::CellBoard& qm, size_t qs);
		protected:
			void paintEvent(QPaintEvent*) override;
		public:
			RectView(QWidget* parent=nullptr);
	};
}
