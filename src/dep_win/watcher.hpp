#pragma once
#include "../domain.hpp"
#include <QRect>
#include <QObject>
#include <unordered_set>
#include <windows.h>
#include <QModelIndex>

Q_DECLARE_METATYPE(HWND)
class QHBoxLayout;
class QStandardItemModel;
namespace dg {
	class Watcher :
		public QObject
	{
		Q_OBJECT
		public:
			Watcher(QObject* parent=nullptr);

			QStandardItemModel* model() const noexcept;
			void startLoop();
			void makeArea(QHBoxLayout* addArea);
		public slots:
			void draggedPosition(QPoint p);
		private slots:
			void _removeRow(QModelIndex idx, int first, int last);
			void _refreshWindowRect();
		signals:
			void onRectChanged(dg::Rect_NameV rect);

		private:
			using HWNDSet = std::unordered_set<HWND>;
			HWNDSet				_wset;
			// Watcherクラスからのみアクセス
			QStandardItemModel*	_model;

			void _makeModelFromWSet();
			bool _cursorOnQtWindow();
	};
}
