#pragma once
#include <QRect>
#include <QObject>
#include <QModelIndex>
#include <mutex>
#include <thread>
#include "x_aux.hpp"
#include "../domain.hpp"

class QHBoxLayout;
class QStandardItemModel;
namespace dg {
	class Watcher :
		public QObject
	{
		Q_OBJECT
		private:
			using WatchId = uint32_t;
			struct WatchEntry {
				WatchId		id;
				WindowD		window;
				Name		name;
				QRect		rect;

				void invalidateRect() noexcept;
				bool isEmpty() const noexcept;
			};
			using WatchEntryV = std::vector<WatchEntry>;

		signals:
			// 監視対象ウィンドウのどれかが変化した
			void onRectChanged(dg::Rect_NameV rect);
		public slots:
			// マウスカーソルでウィンドウを選択してウィンドウ監視
			// ウィンドウが閉じられたら自動で監視対象から削除
			void addWatch();
		private:
			// 指定ウィンドウを監視対象から外す
			void _removeWatch(WatchId id);

		private slots:
			bool _cursorOnQtWindow();
			void _removeRow(QModelIndex idx, int first, int last);
			void _onStateChanged();
			void _onExit(bool error);

		private:
			static uint32_t _EventMask();
			static void _ThreadProc(Watcher* self, int cmdFd) noexcept;
			enum Direction {
				Read = 0,
				Write = 1
			};
			struct Command {
				enum Type {
					AddWatch,
					RemoveWatch,
					Quit,
				};
				Type	type;
				union {
					struct {
					} addwatch;
					struct {
						WindowD		window;
					} removewatch;
				};
			};
			WatchEntry* _findEntry(WindowD w);
			WatchEntry* _findEntry(WatchId id);
			bool _removeEntry(WindowD w);

		public:
			Watcher(QObject* parent=nullptr);
			~Watcher();
			QStandardItemModel* model() const noexcept;
			// Xlibイベントループを開始
			void startLoop();
			void makeArea(QHBoxLayout* addArea);

		private:
			Display_U	_disp;
			// 操作キューにコマンドを追加した時の通知用[Write]
			int			_cmdFd;

			std::thread	_thread;
			mutable std::mutex	_mutex;
			WatchId		_serialId;
			WatchEntryV	_entry;
			// Watcherクラスからのみアクセス
			QStandardItemModel*	_model;
	};
}
