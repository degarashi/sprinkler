#pragma once
#include <QWidget>
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
		public QWidget
	{
		Q_OBJECT
		private:
			QLabel*					_label;
			GFrame*					_frame;
			QString					_path;
			QPersistentModelIndex	_index;
			QMenu*					_ctrlMenu;

			bool _getChecked() const;
		signals:
			void clicked();
		protected:
			void contextMenuEvent(QContextMenuEvent* e) override;
			void mousePressEvent(QMouseEvent* e) override;
		public slots:
			void showLabelFrame(bool b);
		public:
			explicit GLabel(const QString& path, QSize crop,
							const lubee::PointI ofs, QSize resize,
							const QModelIndex& index, QMenu* ctrlMenu);
	};
}
