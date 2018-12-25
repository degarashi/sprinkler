#include "toast_mgr.hpp"

namespace dg {
	ToastMgr::ToastMgr(QObject *const parent):
		QObject(parent),
		_toast(nullptr)
	{}
	void ToastMgr::toastDestroyed() {
		Q_ASSERT(_toast);
		_toast = nullptr;
	}
	const int
		ToastMgr::DefaultFadeIn = 1000,
		ToastMgr::DefaultDuration = 5000,
		ToastMgr::DefaultFadeOut = 1000;
	void ToastMgr::bake(
		const Toast::Icon::e iconType,
		const QString& title,
		const QString& msg,
		const int fadeInMS,
		const int durationMS,
		const int fadeOutMS
	)
	{
		closeToast();

		_toast = new Toast(iconType, title, msg, fadeInMS, durationMS, fadeOutMS);
		_toast->show();
		connect(_toast, &QObject::destroyed,
				this, &ToastMgr::toastDestroyed);
	}
	void ToastMgr::closeToast() {
		if(_toast)
			delete _toast;
	}
	ToastMgr::~ToastMgr() {
		closeToast();
	}
}
