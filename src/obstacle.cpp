#include "obstacle.hpp"
#include <QVariant>
#include <QWindow>

namespace dg {
	Obstacle::Obstacle(QWidget* parent, Qt::WindowFlags flag):
		QWidget(parent, flag),
		_init(false)
	{}

	void Obstacle::showEvent(QShowEvent* e) {
		if(!_init) {
			_init = true;

			// (Quantizerが参照)障害物として記録
			auto* w = windowHandle();
			w->setProperty("obstacle", QVariant::fromValue(true));
		}
		QWidget::showEvent(e);
	}
}
