#include "dragpos.hpp"
#include <QMouseEvent>

namespace dg {
	void DragPos::mouseReleaseEvent(QMouseEvent* e) {
		if(e->button() == Qt::LeftButton) {
			releaseMouse();
			emit dragPosition(QCursor::pos());
		}
	}
	void DragPos::mousePressEvent(QMouseEvent* e) {
		grabMouse(QCursor(Qt::CrossCursor));
	}
}