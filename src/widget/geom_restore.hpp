#pragma once
#include "obstacle.hpp"
#include <QSettings>
#include <QHideEvent>
#include <QDebug>

namespace dg::widget {
	template <class Base>
	class GeomRestore :
		public Obstacle<Base>
	{
		private:
			using base_t = Obstacle<Base>;
			inline static const QString c_key = "geometry";

			QString		_key;
			bool		_shown;

			//! 前回の表示位置を復元
			void _load() {
				QSettings s;
				s.beginGroup(_key);
				base_t::restoreGeometry(s.value(c_key).toByteArray());
			}
			void _save() const {
				QSettings s;
				s.beginGroup(_key);
				s.setValue(c_key, base_t::saveGeometry());
			}
		protected:
			void showEvent(QShowEvent *const e) override {
				// ウィンドウシステム都合ではなくアプリケーション側での初回表示時
				if(!e->spontaneous()) {
					if(!_shown) {
						_shown = true;
						_load();
					}
					_onVisibilityChanged(true);
				}
				base_t::showEvent(e);
			}
			void closeEvent(QCloseEvent *const e) override {
				_save();
				base_t::closeEvent(e);
			}
			void hideEvent(QHideEvent *const e) override {
				if(!e->spontaneous()) {
					_onVisibilityChanged(false);
				}
				base_t::hideEvent(e);
			}
			virtual void _onVisibilityChanged(bool b) = 0;
		public:
			explicit GeomRestore(const QString& key, QWidget *const parent=nullptr):
				base_t(parent, Qt::Window),
				_key(key),
				_shown(false)
			{}
	};
}
