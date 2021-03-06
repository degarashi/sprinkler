#pragma once
#include "spine/src/singleton.hpp"
#include "spine/src/enum.hpp"
#include "idtype.hpp"
#include "place/selected.hpp"
#include "lubee/src/rect.hpp"
#include <QObject>

class QMenu;
class QAction;
namespace dg {
	class SprBoard;
	class CellBoard;
	namespace place {
		struct Param;
		struct Result;
		using ResultV = std::vector<Result>;
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
				size_t			quantify_size,
								delay_ms;
			} _const;

			struct {
				WatchList			*watchList;	// 監視ウィンドウの追加/削除
				RectWindow			*rect;		// 障害物の図示
				ImageDirWindow		*source;
				ImageTagWindow		*tag;
				widget::MainWindow	*mainwin;
			} _window;
			QAction*			_action[Action::_Num];

			void _sprinkle(const place::Param& param, const TagIdV& tag);
			void _sprinkleImageSet(const ImageIdV& id);
			void _resetToIdleState(State::e expected);

			using SprBoard_S = std::shared_ptr<SprBoard>;
			SprBoard_S			_board;

			void _saveBoardState(const CellBoard& board);
			void _removeBoardState();

		private:
			void _initWatchList();
			void _initRectView();
			void _initImageSrc();
			void _initAction();
			void _linkAction();
			// TOMLに記載した値のうち、起動時にしか読み込まれない物をメモリにキャッシュ
			void _storeConstValues();
		public:
			explicit Sprinkler();
			// ---- from MainWindow ----
			// 候補タグと平均枚数を指定して画像を配置
			void sprinkle(const place::Param& param, const TagIdV& tag);
			// 予め用意した画像候補を使って画像を配置
			// (全て配置できるとは限らない)
			void sprinkleImageSet(const ImageIdV& id);
			void abort();
			// -------------------------
			~Sprinkler();
			QAction* getAction(Action::e a) const;
			//! 配置されている画像を右クリックした際に表示するメニュー
			void showImageContextMenu(ImageId id, const QPoint& p);
		signals:
			// 配置計算の進捗通知
			void sprinkleProgress(int p);
			// 配置結果
			void sprinkleResult(const dg::place::ResultV& result);
			// 配置計算のキャンセル確認通知
			void sprinkleAbort();
			// Databaseの画像集合が変更された時に送出
			void imageChanged();
	};
}
