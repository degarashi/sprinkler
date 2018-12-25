#include "checkbox_delegate.hpp"
#include "column_target.hpp"
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>

namespace dg {
	CheckBoxDelegate::CheckBoxDelegate(const ColumnTarget* column, QObject* parent):
		QStyledItemDelegate(parent),
		_column(column)
	{}
	void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
		QStyleOptionViewItem viewItemOption(option);
		// Only do this if we are accessing the column with boolean variables.
		if(_column->has(index.column())) {
			// This basically changes the rectangle in which the check box is drawn.
			const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
			const QRect newRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
				QSize(option.decorationSize.width() + 5,
						option.decorationSize.height()),
				QRect(option.rect.x() + textMargin, option.rect.y(),
					  option.rect.width() - (2 * textMargin), option.rect.height()));
			viewItemOption.rect = newRect;
		}
		// Draw the check box using the new rectangle.
		QStyledItemDelegate::paint(painter, viewItemOption, index);
	}

	bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
		Q_ASSERT(event);
		Q_ASSERT(model);

		// make sure that the item is checkable
		Qt::ItemFlags flags = model->flags(index);
		if(!(flags & Qt::ItemIsUserCheckable) || !(flags & Qt::ItemIsEnabled))
			return false;

		// make sure that we have a check state
		QVariant value = index.data(Qt::CheckStateRole);
		if(!value.isValid())
			return false;

		// make sure that we have the right event type
		if(event->type() == QEvent::MouseButtonRelease) {
			const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
			const QRect checkRect =
				QStyle::alignedRect(option.direction, Qt::AlignCenter,
					option.decorationSize,
					QRect(
						option.rect.x() + (2 * textMargin),
						option.rect.y(),
						option.rect.width() - (2 * textMargin),
						option.rect.height())
					);

			// We handle the mouse event...
			QMouseEvent* mEvent = (QMouseEvent*) event;
			if(!checkRect.contains(mEvent->pos()))
				return false;

		} else if (event->type() == QEvent::KeyPress) {
			if(static_cast<QKeyEvent*>(event)->key() !=
					Qt::Key_Space&& static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select)
				return false;
		} else
			return false;

		// Determine the new check state
		const Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
				? Qt::Unchecked : Qt::Checked);
		// And set the new check state by calling the model's setData() function.
		return model->setData(index, state, Qt::CheckStateRole);
	}
}
