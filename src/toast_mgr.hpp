#pragma once
#include "spine/singleton.hpp"
#include "toast.hpp"

namespace dg {
	#define mgr_toast (::dg::ToastMgr::ref())
	// 現状は1つのToastだけを一度に出せる仕様
	class ToastMgr :
		public QObject,
		public spi::Singleton<ToastMgr>
	{
		Q_OBJECT
		private:
			Toast*		_toast;
		private slots:
			void toastDestroyed();
		public:
			ToastMgr();
			~ToastMgr();
			void closeToast();

			const static int DefaultFadeIn,
							DefaultDuration,
							DefaultFadeOut;
			void bake(
				Toast::Icon::e iconType,
				const QString& title,
				const QString& msg,
				int fadeInMS = DefaultFadeIn,
				int durationMS = DefaultDuration,
				int fadeOutMS = DefaultFadeOut
			);
	};
}
