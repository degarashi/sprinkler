#pragma once
#include "lubee/size.hpp"
#include <QMainWindow>
#include <memory>
#include <unordered_set>
#include "gene_param.hpp"
#include "place_result.hpp"
#include <QSet>

namespace Ui {
	class MainWindow;
}
namespace dg {
	class Watcher;
	class WatchList;
	class DirList;
	class Quantizer;
	class QtWNotifier;
	class ToastMgr;
}
class QModelIndex;
class QSettings;
class QStandardItemModel;
class QStandardItem;
class QSystemTrayIcon;
class QItemSelection;
namespace dg {
	class GeneWorker;
	class GLabel;
	class MainWindow :
		public QMainWindow
	{
		Q_OBJECT
		public:
			explicit MainWindow(QWidget* parent=nullptr);
		private:
			const static size_t QuantifySize;
			struct Request {
				enum {
					Max,
					Min,
					Sample,
					_Num
				};
			};
			using LabelV = std::vector<GLabel*>;
			using UI_S = std::shared_ptr<Ui::MainWindow>;
			using Toast_S = std::shared_ptr<ToastMgr>;

		private:
			UI_S				_ui;
			Toast_S				_toast;
			dg::Watcher			*_watcher;		// XWindowによるウィンドウ位置監視
			dg::Quantizer		*_quantizer;	// ウィンドウ位置変更を検知、量子化
			WatchList			*_watchList;	// 監視ウィンドウダイアログ
			QStandardItemModel	*_dirModel,		// [1>Display: フルパス(QString), 1>User: ファイルリスト(PathV), 2>Display: ファイルカウント(int)]
								*_reqModel,
								*_keepModel,	// [Display: ファイル名(QString), User: フルパス(QString), Decoration: サムネイル(QPixmap)]
								// 画像の配置状態モデル
								// [1>Display:(none), User:State-Index, Decoration: サムネイル(QPixmap)]
								*_stateModel;
			PathS				_keepSet;
			// [1: DirectoryPath() + Size_PathV(User), 2: Image-Count]
			DirList				*_dirList;		// 画像フォルダダイアログ
			// Label: 配置した画像(QLabel)
			LabelV				_label;
			// 全画像セット(shown + notshown)にサイズを対応付けた物
			ImageSet			_imageSet;
			PathS				_shown,
								_notshown,
								_shownP,
								_notshownP;
			QThread*			_workerThread;
			GeneWorker*			_geneWorker;
			QtWNotifier*		_qtntf;
			QSystemTrayIcon*	_tray;
			QAction*			_actionShow;
			QMenu*				_ctrlMenu;
			bool				_obstacle;
			using StateV = std::vector<PlaceV>;
			// 画像の配置状態リスト
			StateV				_state;

			void _initDirModel();
			void _initDirList();
			void _initWatchList();
			void _initRequestModel();
			void _initKeepModel();
			void _initSystemTray();
			void _initStateModel();
			void _saveDirModel(QSettings& s);
			void _saveKeepModel(QSettings& s);
			// 表示中のラベルを全て削除
			void _clearLabels();
			void _emitSprinkleCounterChanged();
			void _removeDirItem(QStandardItem* item);
			void _setReqData(int index, const QVariant& v);
			QVariant _getReqData(int index) const;
			void _sprinkle();
			void _setControlsEnabled(bool b);
			void _saveInfo();
			void _pushState(const PlaceV& state);
			void _applyState(const PlaceV& state);
			QPixmap _makeThumbnail(const PlaceV& state);
			static void _CollectImageInDir(PathV& imgv, ImageSet& imgs, const QString& path, bool recursive);
			static void _CollectImage(PathV& imgv, ImageSet& imgs, const QString& path);
		public:
			size_t stateCount() const noexcept;

		signals:
			void sprinkleCounterChanged(size_t shown, size_t notshown);
			void onProgress(int p);
			void showLabelFrame(bool b);
			void keepChanged(const QString& path, bool b);
			void stateChanged(int idx);

		private slots:
			// ---- from WatchList signal ----
			void watchListClosed();
			// ---- from DirList signal ----
			void dirListClosed();
			// ---- from DirModel signal ----
			// DirModelのアイテムに対応するサイズリストを構築 -> SizeMに格納
			void syncImageSizeList(QStandardItem* item);
			void dirRemoving(const QModelIndex& index, int first, int last);
			void removeKeep();
			void removeKeepAll();
			void showWindow(bool b);
			void setKeep(const QString& path, bool b);
			void keepRemoving(const QModelIndex& index, int first, int last);
			void stateSelect(const QItemSelection& sel);

		public slots:
			void showWatchList(bool show);
			void showDirList(bool show);
			void stayOnTop(bool top);

			void sprinkleReset();
			void sprinkle();
			void reSprinkle();
			void receiveResult(const dg::PlaceV& place);
			void setState(size_t n);
			void setLastState();

		protected:
			void closeEvent(QCloseEvent* e) override;
			void changeEvent(QEvent* e) override;
			void showEvent(QShowEvent* e) override;
	};
}
