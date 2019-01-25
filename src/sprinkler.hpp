#pragma once
#include "spine/src/singleton.hpp"
#include "spine/src/enum.hpp"
#include "idtype.hpp"
#include "place/selected.hpp"
#include <QObject>

class QMenu;
class QAction;
namespace dg {
	class CellBoard;
	namespace place {
		struct Param;
		struct Result;
		using ResultV = QVector<Result>;
	}
	class ToastMgr;
	class Watcher;
	class Quantizer;
	class QtWNotifier;
	class WatchList;
	class RectWindow;
	class ImageDirWindow;
	class ImageTagWindow;
	class Database;
	class GeneWorker;
	namespace widget {
		class MainWindow;
	}

	/*
		スレッド

		/ImageDirロード
		/サムネイル生成
		/配置計算
	*/
	#define sprinkler	(::dg::Sprinkler::ref())
	class Sprinkler :
		public QObject,
		public spi::Singleton<Sprinkler>
	{
		Q_OBJECT
		public:
			struct Action {
				enum e {
					OpenDir,
					OpenTag,
					OpenRect,
					OpenWatch,
					OpenMain,
					Quit,
					ResetFlag,
					_Num
				};
			};
			DefineEnum(State,
				(Idle)			// 処理受付中
				(WaitDelay)		// Label消去後のProcessing待ち
				(Processing)	// 配置計算中
				(Aborted)		// 処理中断、スレッド終了待ち
			);
		private:
			ToastMgr			*_toast;
			Watcher				*_watcher;		// ウィンドウ位置監視
			Quantizer			*_quantizer;	// ウィンドウ位置変更を検知、量子化
			QtWNotifier			*_qtntf;		// Qtウィンドウの移動監視
			Database			*_db;
			QThread				*_workerThread;
			GeneWorker			*_geneWorker;
			State				_state;

			struct {
				WatchList			*watchList;	// 監視ウィンドウの追加/削除
				RectWindow			*rect;		// 障害物の図示
				ImageDirWindow		*source;
				ImageTagWindow		*tag;
				widget::MainWindow	*mainwin;
			} _window;
			QAction*			_action[Action::_Num];

			void _sprinkle(const place::Param& param, const TagIdV& tag);

		private:
			void _initWatchList();
			void _initRectView();
			void _initImageSrc();
			void _initAction();
			void _linkAction();
		public:
			explicit Sprinkler();
			// from MainWindow
			void sprinkle(const place::Param& param, const TagIdV& tag);
			void abort();
			~Sprinkler();
			QAction* getAction(Action::e a) const;
			void showTagMenu(ImageId id, const QPoint& p);
		signals:
			void sprinkleProgress(int p);
			void sprinkleResult(const dg::place::ResultV& result);
			void sprinkleAbort();
			void imageChanged();
	};
}
