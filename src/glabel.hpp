#pragma once
#include <QLabel>
#include <QPersistentModelIndex>
#include "lubee/size.hpp"
#include "lubee/point.hpp"
#include "keepdata.hpp"

namespace dg {
	class GLabel :
		public QLabel
	{
		Q_OBJECT
		private:
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
