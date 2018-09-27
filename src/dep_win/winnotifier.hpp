#pragma once
#include <QObject>
#include "ghook/wevent.hpp"

class QTimer;
class WinNotifier :
	public QObject
{
	Q_OBJECT
	public:
		explicit WinNotifier(QObject* parent = nullptr);
		~WinNotifier();
    private slots:
		void onNotify();
	signals:
		void onEvent();
	private:
		QThread*	_th;
		QTimer*		_timer;
		WEvent		_event32,
					_event64;
};
