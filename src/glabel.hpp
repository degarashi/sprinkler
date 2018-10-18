#pragma once
#include "obstacle.hpp"
#include "lubee/src/size.hpp"
#include "lubee/src/point.hpp"

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
			QMenu*					_ctrlMenu;
			QPoint					_offset;
			bool					_keep;

			bool _getChecked() const;
		signals:
			void clicked();
			void keepChanged(const QString& path, bool b);
		protected:
			void contextMenuEvent(QContextMenuEvent* e) override;
			void mousePressEvent(QMouseEvent* e) override;
			void moveEvent(QMoveEvent* e) override;
		public slots:
			void showLabelFrame(bool b);
			void setKeep(const QString& path, bool b);
		public:
			explicit GLabel(const QString& path, QSize crop,
							const lubee::PointI ofs, QSize resize,
							bool keep, QMenu* ctrlMenu);
			const QString& path() const noexcept;
			const QPixmap* pixmap() const;
	};
}
