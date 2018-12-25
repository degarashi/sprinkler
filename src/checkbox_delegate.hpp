#pragma once
#include <QStyledItemDelegate>

namespace dg {
	class ColumnTarget;
	class CheckBoxDelegate :
		public QStyledItemDelegate
	{
		Q_OBJECT
		private:
			const ColumnTarget*		_column;
		public:
			CheckBoxDelegate(const ColumnTarget* column, QObject* parent=nullptr);
			void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
			bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
	};
}
