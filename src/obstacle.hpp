#pragma once
#include <QWidget>

namespace dg {
	class Obstacle :
		public QWidget
	{
		Q_OBJECT

		private:
			bool	_init;
		protected:
			void showEvent(QShowEvent* e) override;
		public:
			Obstacle(QWidget* parent = nullptr, Qt::WindowFlags flag = 0);
	};
}
