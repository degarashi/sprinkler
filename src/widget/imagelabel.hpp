#pragma once
#include "obstacle.hpp"
#include "lubee/src/size.hpp"
#include "lubee/src/point.hpp"
#include <QWidget>

class QLabel;
class QMenu;
// namespace dg::widgetとするとメタコンパイラがtr()の解釈でコケるので…
namespace dg { namespace widget {
	class ColorFrame;
	//! 配置計算した画像の表示
	class ImageLabel :
		public Obstacle<QWidget>
	{
		Q_OBJECT
		private:
			QLabel*					_label;
			ColorFrame*				_frame;
			QString					_path;
			QMenu*					_ctrlMenu;
			QPoint					_offset;

		signals:
			void clicked();
		protected:
			void contextMenuEvent(QContextMenuEvent* e) override;
			void mousePressEvent(QMouseEvent* e) override;
			void moveEvent(QMoveEvent* e) override;
		public:
			explicit ImageLabel(const QString& path, QSize crop,
							const lubee::PointI ofs, QSize resize,
							QMenu* ctrlMenu);
			const QString& path() const noexcept;
			const QPixmap* pixmap() const;
	};
}}
