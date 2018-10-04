#pragma once
#include <QWidget>
#include <QPersistentModelIndex>
#include "lubee/size.hpp"
#include "lubee/point.hpp"
#include "keepdata.hpp"

class QLabel;
namespace dg {
	class GLabel :
		public QWidget
	{
		Q_OBJECT
		private:
			QLabel*					_label;
			QString					_path;
			QPersistentModelIndex	_index;

			bool _getChecked() const;
		signals:
			void clicked();
		protected:
			void contextMenuEvent(QContextMenuEvent* e) override;
			void mousePressEvent(QMouseEvent* e) override;
		public:
			explicit GLabel(const QString& path, QSize crop,
							const lubee::PointI ofs, QSize resize,
							const QModelIndex& index);
	};
}
