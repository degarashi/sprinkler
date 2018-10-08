#pragma once
#include "obstacle.hpp"
#include <QPersistentModelIndex>
#include "lubee/size.hpp"
#include "lubee/point.hpp"
#include "keepdata.hpp"

class QLabel;
class QMenu;
namespace dg {
	class GFrame :
		public QWidget
	{
		Q_OBJECT
		protected:
			void paintEvent(QPaintEvent* e) override;
		public:
			GFrame(QWidget* parent = nullptr);
	};

	class GLabel :
		public Obstacle
	{
		Q_OBJECT
		private:
			QLabel*					_label;
			GFrame*					_frame;
			QString					_path;
			QPersistentModelIndex	_index;
			QMenu*					_ctrlMenu;
			QPoint					_offset;

			bool _getChecked() const;
		signals:
			void clicked();
		protected:
			void contextMenuEvent(QContextMenuEvent* e) override;
			void mousePressEvent(QMouseEvent* e) override;
			void moveEvent(QMoveEvent* e) override;
		public slots:
			void showLabelFrame(bool b);
		public:
			explicit GLabel(const QString& path, QSize crop,
							const lubee::PointI ofs, QSize resize,
							const QModelIndex& index, QMenu* ctrlMenu);
			const QString& path() const noexcept;
			const QPixmap* pixmap() const;
	};
}
