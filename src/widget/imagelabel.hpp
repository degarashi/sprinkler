#pragma once
#include "obstacle.hpp"
#include "lubee/src/rect.hpp"
#include "../idtype.hpp"
#include <QWidget>

class QLabel;
class QMenu;
namespace dg {
	struct DBImage;
	struct DBTag;
}
// namespace dg::widgetとするとメタコンパイラがtr()の解釈でコケるので…
namespace dg { namespace widget {
	class ColorFrame;
	//! 配置計算した画像の表示
	class ImageLabel :
		public Obstacle<QWidget>
	{
		Q_OBJECT
		private:
			ImageId					_id;
			QLabel*					_label;
			ColorFrame*				_frame;
			QMenu*					_ctrlMenu;
			QPoint					_offset;
			const DBImage*			_dbImage;
			const DBTag*			_dbTag;

		signals:
			void clicked();
		protected:
			void contextMenuEvent(QContextMenuEvent* e) override;
			void mousePressEvent(QMouseEvent* e) override;
			void moveEvent(QMoveEvent* e) override;
		public:
			explicit ImageLabel(ImageId id,
							const lubee::RectI& rect,
							QMenu* ctrlMenu,
							const DBImage* dbImage, const DBTag* dbTag);
	};
}}
