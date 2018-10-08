#pragma once
#include "obstacle.hpp"

namespace dg {
	class GeomRestore :
		public Obstacle
	{
		Q_OBJECT

		private:
			QString		_key;
			bool		_first;
		protected:
			void closeEvent(QCloseEvent* e) override;
			void showEvent(QShowEvent* e) override;
		signals:
			void onClose();
		public:
			explicit GeomRestore(const QString& key, QWidget* parent=nullptr);
	};
}
