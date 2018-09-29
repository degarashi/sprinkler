#pragma once
#include "lubee/size.hpp"
#include <QMainWindow>
#include <memory>
#include <unordered_set>
#include <QModelIndex>
#include "gene_param.hpp"

namespace Ui {
	class MainWindow;
}
namespace dg {
	class Watcher;
	class WatchList;
	class DirList;
	class Quantizer;
	class QtWNotifier;
}
class QSettings;
class QStandardItemModel;
class QStandardItem;
class QSystemTrayIcon;
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
					PreferOriginal,
					_Num
				};
			};
			using LabelV = std::vector<GLabel*>;
			using UI_S = std::shared_ptr<Ui::MainWindow>;

		private:
			UI_S				_ui;
			dg::Watcher			*_watcher;		// XWindowによるウィンドウ位置監視
			dg::Quantizer		*_quantizer;	// ウィンドウ位置変更を検知、量子化
			WatchList			*_watchList;	// 監視ウィンドウダイアログ
			QStandardItemModel	*_dirModel,		// 画像フォルダデータ実体
								*_reqModel,
								*_keepModel;
			// [1: DirectoryPath() + Size_PathV(User), 2: Image-Count]
			DirList				*_dirList;		// 画像フォルダダイアログ
			// Label: 配置した画像(QLabel)
			LabelV				_label;
			ImageSet			_shown,
								_notshown,
								_shownP,
								_notshownP;
			QThread*			_workerThread;
			GeneWorker*			_geneWorker;
			QtWNotifier*		_qtntf;
			QSystemTrayIcon*	_tray;
			// [path -> modelindex]
			QHash<QString, QModelIndex> _path2idx;

			void _initDirModel();
			void _initDirList();
			void _initWatchList();
			void _initRequestModel();
			void _initKeepModel();
			void _initSystemTray();
			void _cleanKeepList();
			void _clearKeepList();
			void _saveDirModel(QSettings& s);
			void _saveKeepModel(QSettings& s);
			// 表示中のラベルを全て削除
			void _clearLabels();
			void _emitSprinkleCounterChanged();
			void _removeImage(QStandardItem* item);
			void _setReqData(int index, const QVariant& v);
			QVariant _getReqData(int index) const;
			void _sprinkle();
			void _setControlsEnabled(bool b);
			static void _CollectImageInDir(ImageV& imgv, ImageSet& imgs, const QString& path, bool recursive);
			static void _CollectImage(ImageV& imgv, ImageSet& imgs, const QString& path);

		signals:
			void sprinkleCounterChanged(size_t shown, size_t notshown);
			void onProgress(int p);

		private slots:
			// ---- from WatchList signal ----
			void watchListClosed();
			// ---- from DirList signal ----
			void dirListClosed();
			// ---- from DirModel signal ----
			// DirModelのアイテムに対応するサイズリストを構築 -> SizeMに格納
			void syncImageSizeList(QStandardItem* item);
			void dirRemoving(QModelIndex index, int first, int last);
			void removeKeep();

		public slots:
			void showWatchList(bool show);
			void showDirList(bool show);
			void stayOnTop(bool top);

			void sprinkleReset();
			void sprinkle();
			void reSprinkle();
			void receiveResult(const dg::PlaceV& place);

		protected:
			void closeEvent(QCloseEvent* e) override;
	};
}
