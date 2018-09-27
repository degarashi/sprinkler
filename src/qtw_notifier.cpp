#include "qtw_notifier.hpp"
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QWidget>
#include <QTimer>

namespace dg {
	namespace {
		constexpr int DelayInterval = 200;
	}
	QtWNotifier::QtWNotifier(QObject *const parent):
		QObject(parent),
		_delayTimer(new QTimer(this))
	{
		_delayTimer->setInterval(DelayInterval);
		connect(_delayTimer, &QTimer::timeout,
				this, [this](){
					_delayTimer->stop();
					emit onQtGeometryChanged();
				}
		);
		// スクリーンのサイズが変わったり、追加削除があった時には通知を受け取る
		for(QScreen *const s : qApp->screens())
			connect(s, SIGNAL(geometryChanged(QRect)), this, SLOT(onScreenGeometryChanged()));
		connect(qApp, SIGNAL(screenAdded(QScreen*)), this, SLOT(onScreenAdded(QScreen*)));
		connect(qApp, SIGNAL(screenRemoved(QScreen*)), this, SLOT(onScreenRemoved(QScreen*)));

		// Qtのウィンドウを監視
		qApp->installEventFilter(this);
	}
	bool QtWNotifier::eventFilter(QObject *const obj, QEvent *const e) {
		if(!obj->parent()) {
			// Qtの管理するウィンドウがどれか動いたらupdateをかける
			if(QWidget *const w = qobject_cast<QWidget*>(obj)) {
				switch(e->type()) {
					case QEvent::Resize:
					case QEvent::Move:
					case QEvent::Hide:
					case QEvent::Show:
					case QEvent::Close:
						_delayTimer->start();
						break;
					default:
						break;
				}
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
