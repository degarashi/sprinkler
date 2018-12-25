#pragma once
#include <QVariant>
#include <QWindow>

namespace dg::widget {
	class ObstacleBase {
		protected:
			inline static const char* EntryName = "obstacle";
		public:
			static bool IsObstacle(const QObject*const obj) {
				const QVariant v = obj->property(EntryName);
				return v.isValid() && v.toBool();
			}
	};
	template <class Base>
	class Obstacle :
		public Base,
		public ObstacleBase
	{
		private:
			using base_t = Base;

			bool	_init;
		protected:
			void showEvent(QShowEvent* e) override {
				if(!_init) {
					_init = true;

					// (Quantizerが参照)障害物として記録
					auto* w = base_t::windowHandle();
					w->setProperty(EntryName, QVariant::fromValue(true));
				}
				base_t::showEvent(e);
			}
		public:
			Obstacle(QWidget* parent=nullptr, Qt::WindowFlags flag=0):
				base_t(parent, flag),
				_init(false)
			{}
	};
}
