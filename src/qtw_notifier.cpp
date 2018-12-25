#include "qtw_notifier.hpp"
#include "widget/obstacle.hpp"
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QWidget>
#include <QTimer>

namespace dg {
	QtWNotifier::QtWNotifier(const size_t delayMS, QObject *const parent):
		QObject(parent),
		_delayTimer(new QTimer(this))
	{
		_delayTimer->setInterval(static_cast<int>(delayMS));
		connect(_delayTimer, &QTimer::timeout,
				this, [this](){
					_delayTimer->stop();
					qDebug() << "QtWNotifier: onQtGeometryChanged";
					emit onQtGeometryChanged();
				}
		);
		// スクリーンのサイズが変わったり、追加削除があった時には通知を受け取る
		for(QScreen *const s : qApp->screens()) {
			connect(s, &QScreen::geometryChanged,
					this, &QtWNotifier::onScreenGeometryChanged);
		}
		connect(qApp, &QGuiApplication::screenAdded,
				this, &QtWNotifier::onScreenAdded);
		connect(qApp, &QGuiApplication::screenRemoved,
				this, &QtWNotifier::onScreenRemoved);

		// Qtのウィンドウを監視
		qApp->installEventFilter(this);
	}
	bool QtWNotifier::eventFilter(QObject *const obj, QEvent *const e) {
		// Qtの管理するウィンドウがどれか動いたらupdateをかける
		if(widget::ObstacleBase::IsObstacle(obj)) {
			switch(e->type()) {
				case QEvent::Resize:
				case QEvent::Move:
				case QEvent::Show:
				case QEvent::Hide:
				case QEvent::Expose:
				case QEvent::Close:
				case QEvent::WindowTitleChange:
				case QEvent::WindowStateChange:
					_delayTimer->start();
					break;
				default:
					break;
			}
		}
		return false;
	}
	void QtWNotifier::onScreenAdded(QScreen* scr) {
		qDebug() << "QtWNotifier: onScreenAdded" << scr;
		emit onQtGeometryChanged();
	}
	void QtWNotifier::onScreenRemoved(QScreen* scr) {
		qDebug() << "QtWNotifier: onScreenRemoved" << scr;
		emit onQtGeometryChanged();
	}
	void QtWNotifier::onScreenGeometryChanged() {
		qDebug() << "QtWNotifier: onScreenGeometryChanged";
		emit onQtGeometryChanged();
	}
}
