#pragma once
#include <QLabel>

namespace dg {
	class DragPos :
		public QLabel
	{
		Q_OBJECT
		signals:
			void dragPosition(QPoint p);
		protected:
			void mouseReleaseEvent(QMouseEvent* e) override;
			void mousePressEvent(QMouseEvent* e) override;
		public:
			using QLabel::QLabel;
	};
}
