#pragma once
#include <QWidget>

namespace dg::widget {
	//! 単色フレーム枠
	class ColorFrame :
		public QWidget
	{
		Q_OBJECT
		private:
			int		_lineWidth;
		protected:
			void paintEvent(QPaintEvent* e) override;
		public:
			ColorFrame(QWidget* parent = nullptr);
			void setWidth(int w);
	};
}
