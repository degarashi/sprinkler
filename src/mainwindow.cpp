#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "watchlist.hpp"
#include "watcher.hpp"
#include "rectwindow.hpp"
#include "quantizer.hpp"
#include "qtw_notifier.hpp"
#include "dirlist.hpp"
#include "histgram/src/nboard.hpp"
#include "histgram/src/maxrect.hpp"
#include "boolarray.hpp"
#include <QStandardItemModel>
#include <QSettings>
#include <QDir>
#include <QLabel>
#include <QImageReader>
#include <QDebug>
#include <QCheckBox>
#include <QThread>
#include <QTimer>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include "glabel.hpp"
#include "gene_worker.hpp"
#include "version.hpp"
#include <QWindow>
#include "toast_mgr.hpp"
#include <QScreen>
#include "aux.hpp"
#include <QPainter>

namespace dg {
	namespace {
		QString c_mainwindow("MainWindow"),
				c_dirmodel("DirModel"),
				c_keepmodel("KeepModel");
	}
	void MainWindow::setLastState() {
		setState(stateCount() - 1);
	}
	size_t MainWindow::stateCount() const noexcept {
		return _state.size();
	}
	void MainWindow::setState(const size_t n) {
		_clearLabels();
		_applyState(_state.at(n));
		emit stateChanged(n);
	}
	void MainWindow::_pushState(const State& state) {
		_state.emplace_back(state);

		// MainWindow -> Modelの同期
		QStandardItem* item = new QStandardItem;
		item->setData(QString("%1 Images").arg(state.place.size()), Qt::EditRole);
		QPixmap thumbnail = _makeThumbnail(state);
		item->setData(thumbnail, Qt::DecorationRole);
		_stateModel->appendRow(item);
	}
	QPixmap MainWindow::_makeThumbnail(const State& state) {
		const QSize vsize0 = qApp->primaryScreen()->availableVirtualSize();
		const QSize vsize = AspectKeepScale({128,128}, vsize0);
		const float r = float(vsize.width()) / vsize0.width();
		const QSizeF sizeR{r, r};
		QPixmap ret(vsize);
		ret.fill(Qt::gray);
		QPainter painter(&ret);
		// 配置された画像の縮図
		for(auto& p : state.place) {
			QImageReader reader(p.path);
			reader.setScaledSize(ToQSize(p.resize * sizeR));

			QPixmap pix = QPixmap::fromImage(reader.read());
			QPoint ofs{
				static_cast<int>(p.offset.x * r),
				static_cast<int>(p.offset.y * r)
			};
			painter.drawPixmap(QRect{ofs, pix.size()}, pix);
		}
		if(isWindowShowing()) {
			// メインウィンドウの縮図
			QWidget* cw = centralWidget();
			QPixmap main = cw->grab();
			main = main.scaled(ToQSize(cw->size() * sizeR));
			const QPointF gpos(cw->mapToGlobal(cw->pos()));
			painter.drawPixmap(QRect{
				ToQPoint(gpos * sizeR),
				ToQSize(QSizeF(cw->size()) * sizeR)
			}, main);
		}
		return ret;
	}
	void MainWindow::_applyState(const State& state) {
		for(auto& p : state.place) {
			{
				GLabel* lb =
					new GLabel(
						p.path,
						p.crop,
						p.offset,
						p.resize,
						_keepSet.find(p.path) != _keepSet.end(),
						_ctrlMenu
					);
				// どれか1つをクリックしたら他の全てのGLabelと自分を前面に持ってくる
				connect(lb, &GLabel::clicked, this, [this](){
					for(auto* l : _label)
						l->raise();
					raise();
				});
				connect(this, SIGNAL(showLabelFrame(bool)), lb, SLOT(showLabelFrame(bool)));
				connect(this, SIGNAL(keepChanged(QString,bool)), lb, SLOT(setKeep(QString,bool)));
				connect(lb, SIGNAL(keepChanged(QString,bool)), this, SLOT(setKeep(QString,bool)));
				_label.emplace_back(lb);
			}
			auto itr = _notshown.find(p.path);
			if(itr != _notshown.end()) {
				_shown.insert(*itr);
				_notshown.erase(itr);
			}
		}
		showWindow(state.showMain);
		if(state.showMain)
			setGeometry(state.mainRect);
	}
	void MainWindow::receiveResult(const PlaceV& place) {
		QString title,
				msg;
		if(place.empty()) {
			title = tr("No image");
			msg = tr("There's no image can place");
		} else {
			title = tr("Image placed");
			msg = tr("%n image(s) placed", "", place.size());
			_pushState(
				State{
					.place = place,
					.showMain = isWindowShowing(),
					.mainRect = this->geometry()
				}
			);
			setLastState();
		}
		mgr_toast.bake(
			Toast::Icon::Information,
			title,
			msg
		);
		_emitSprinkleCounterChanged();
		_setControlsEnabled(true);
	}
	void MainWindow::setKeep(const QString& path, const bool b) {
		auto* m = _keepModel;
		const auto itr = _keepSet.find(path);
		if(b) {
			if(itr != _keepSet.end())
				return;
			_keepSet.insert(path);

			m->appendRow(new QStandardItem);
			const QModelIndex idx = m->index(m->rowCount()-1, 0);
			m->setData(idx, QFileInfo(path).fileName(), Qt::EditRole);
			m->setData(idx, path, Qt::UserRole);
			// サムネイル生成
			QImage img(path);
			m->setData(
				idx,
				QPixmap::fromImage(img.scaled({64,64}, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
				Qt::DecorationRole
			);
		} else {
			if(itr == _keepSet.end())
				return;
			_keepSet.erase(itr);

			const int nk = m->rowCount();
			for(int i=0 ; i<nk ; i++) {
				if(m->item(i)->data(Qt::UserRole).toString() == path) {
					m->removeRow(i);
					break;
				}
			}
		}
		emit keepChanged(path, b);
		emit showLabelFrame(true);
	}
	void MainWindow::showWindow(const bool b) {
		if(b) {
			show();
			setWindowState(Qt::WindowNoState);
		} else {
			setWindowState(Qt::WindowMinimized);
			hide();
		}
		_actionShow->setChecked(b);
	}
	bool MainWindow::isWindowShowing() const {
		return windowState() == Qt::WindowNoState;
	}
	void MainWindow::_setControlsEnabled(const bool b) {
		_ui->pbCurrent->setEnabled(b);
		_ui->pbInit->setEnabled(b);
		_ui->pbNext->setEnabled(b);

		_ui->slMax->setEnabled(b);
		_ui->slMin->setEnabled(b);
		_ui->slSamp->setEnabled(b);

		_ui->listKeep->setEnabled(b);
		_ui->removeKeep->setEnabled(b);
		_ui->removeKeepAll->setEnabled(b);
		_ui->cbHideSprinkle->setEnabled(b);

		_tray->setIcon(QApplication::style()->standardIcon(
			b ? QStyle::SP_TitleBarMenuButton : QStyle::SP_BrowserReload));
	}
	void MainWindow::_initSystemTray() {
		if(QSystemTrayIcon::isSystemTrayAvailable()) {
			_tray = new QSystemTrayIcon(this);
			_tray->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarMenuButton));
			connect(_tray, &QSystemTrayIcon::activated, this, [this](const QSystemTrayIcon::ActivationReason reason){
				switch(reason) {
					case QSystemTrayIcon::Context:
					{
						// 詳細メニュー
						QMenu menu(this);
						menu.addSection(tr("detail menu"));
						menu.addAction(_actionShow);
						menu.addSeparator();
						menu.addAction(_ui->actionOpenWatchList);
						menu.addAction(_ui->actionOpenDirList);
						menu.addSeparator();
						menu.addAction(_ui->actionSprinkle);
						menu.addAction(_ui->actionRe_Sprinkle);
						menu.addAction(_ui->actionReset);
						menu.addSeparator();
						menu.addAction(_ui->actionQuit);
						menu.exec(QCursor::pos());
						break;
					}
					case QSystemTrayIcon::Trigger:
					{
						// 簡易メニュー
						QMenu menu(this);
						menu.addSection(tr("simplified menu"));
						menu.addAction(_actionShow);
						menu.addSeparator();
						menu.addAction(_ui->actionSprinkle);
						menu.addAction(_ui->actionRe_Sprinkle);
						menu.exec(QCursor::pos());
						break;
					}
					default: break;
				}
			});
			_tray->show();
			// タスクトレイをホバーすると今どの程度のイメージが表示されたのかの割合を表示する
			connect(this, &MainWindow::sprinkleCounterChanged,
					this, [this](const size_t shown, const size_t notshown){
				_tray->setToolTip(QString(tr("[%1 / %2] were displayed")).arg(shown).arg(shown + notshown));
			});
		}
	}
	void MainWindow::_initStateModel() {
		_stateModel = new QStandardItemModel(0, 1, this);
		_ui->listState->setModel(_stateModel);
		QListView* lview = _ui->listState;
		// 項目をクリックしたらそのステートを読み込み
		connect(
			lview->selectionModel(),
			SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
			this,
			SLOT(stateSelect(QItemSelection))
		);
		// setStateしたらViewにそれを反映
		connect(
			this,
			&MainWindow::stateChanged,
			lview,
			[lview](const int idx){
				QItemSelectionModel* sel = lview->selectionModel();
				sel->setCurrentIndex(sel->model()->index(idx, 0), QItemSelectionModel::ClearAndSelect);
			}
		);

		// セーブされたステートのロードなど
	}
	void MainWindow::stateSelect(const QItemSelection& sel) {
		if(sel.empty())
			return;
		const QModelIndexList lst = sel.indexes();
		Q_ASSERT(lst.size() == 1);
		setState(lst[0].row());
	}
	MainWindow::MainWindow(QWidget *const parent):
		QMainWindow(parent),
		_ui(new Ui::MainWindow),
		_toast(new ToastMgr),
		_watchList(nullptr),
		_watcher(nullptr),
		_dirModel(nullptr),
		_reqModel(nullptr),
		_keepModel(nullptr),
		_stateModel(nullptr),
		_dirList(nullptr),
		_tray(nullptr),
		_actionShow(nullptr),
		_obstacle(false)
	{
		_ui->setupUi(this);
		{
			// バージョン番号の表示
			const auto ver = QString("Sprinkler (v%1)").arg(QString::fromStdString(Version::GetString()));
			setWindowTitle(ver);
		}
		connect(qApp, &QGuiApplication::focusWindowChanged, this, [this](QWindow* w){
			// 何かウィンドウが選択されている時、キープされている画像は枠を表示
			emit showLabelFrame(static_cast<bool>(w));
		});

		try {
			_initDirModel();
			_initDirList();
			_initWatchList();
			_initRequestModel();
			_initKeepModel();
			_initSystemTray();
			_initStateModel();

			// ウィンドウサイズ復帰
			_actionShow = new QAction(this);
			_actionShow->setCheckable(true);
			_actionShow->setChecked(true);
			_actionShow->setText(tr("Show Window"));
			connect(_actionShow, SIGNAL(toggled(bool)), this, SLOT(showWindow(bool)));

			_workerThread = new QThread(this);
			_workerThread->start();
			_geneWorker = new GeneWorker;
			_geneWorker->moveToThread(_workerThread);
			qRegisterMetaType<dg::PlaceV>("dg::PlaceV");
			connect(_geneWorker, SIGNAL(onProgress(int)), this, SIGNAL(onProgress(int)));
			connect(_geneWorker, SIGNAL(geneResult(dg::PlaceV)), this, SLOT(receiveResult(dg::PlaceV)));

			// ウィンドウの表示状態を復元
			QSettings s;
			s.beginGroup(c_mainwindow);
			restoreGeometry(s.value("geometry").toByteArray());
			if(s.value("showWatchList", false).toBool()) {
				_ui->actionOpenWatchList->setChecked(true);
			}
			if(s.value("showDirList", false).toBool()) {
				_ui->actionOpenDirList->setChecked(true);
			}
			if(s.value("stayOnTop", false).toBool()) {
				_ui->actionStayOnTop->setChecked(true);
			}
			_emitSprinkleCounterChanged();
		} catch(const std::exception& e) {
			QMetaObject::invokeMethod(this, [msg=QString(e.what()), this](){
				QMessageBox::warning(
					this,
					tr("Error occurred"),
					msg
				);
				qApp->quit();
			}, Qt::QueuedConnection);
		}
		_pushState(
			State{
				.place = {},
				.showMain = true,
				.mainRect = this->geometry()
			}
		);

		_ctrlMenu = new QMenu(this);
		_ctrlMenu->addAction(_actionShow);
		_ctrlMenu->addAction(_ui->actionOpenDirList);
		_ctrlMenu->addAction(_ui->actionOpenWatchList);
		_ctrlMenu->addSeparator();
		_ctrlMenu->addAction(_ui->actionSprinkle);
		_ctrlMenu->addAction(_ui->actionRe_Sprinkle);
		_ctrlMenu->addSeparator();
		_ctrlMenu->addAction(_ui->actionQuit);
	}
	void MainWindow::_setReqData(const int index, const QVariant& v) {
		_reqModel->setData(_reqModel->index(index, 0), v);
	}
	QVariant MainWindow::_getReqData(const int index) const {
		return _reqModel->data(_reqModel->index(index, 0), Qt::EditRole);
	}
	void MainWindow::_initRequestModel() {
		Q_ASSERT(!_reqModel);
		_reqModel = new QStandardItemModel(Request::_Num, 1, this);
		auto* m = _reqModel;
		{
			auto* mag = _ui->slMax;
			mag->setName(tr("Max"));
			mag->refSlider()->setRange(10, 100);
			mag->setModel(m, Request::Max);
			_setReqData(Request::Max, 0.6);
		}
		{
			auto* min = _ui->slMin;
			min->setName(tr("Min"));
			min->refSlider()->setRange(10, 100);
			min->setModel(m, Request::Min);
			_setReqData(Request::Min, 0.3);
		}
		{
			auto* samp = _ui->slSamp;
			samp->setName(tr("Samp"));
			samp->refSlider()->setRange(1, 8);
			samp->setModel(m, Request::Sample);
			_setReqData(Request::Sample, 4);
		}
		connect(_reqModel, &QAbstractItemModel::dataChanged,
				this, [this](const QModelIndex& lt, const QModelIndex& rb, const QVector<int>& role){
					Q_UNUSED(rb);
					Q_UNUSED(role);
					const float min = _getReqData(Request::Min).toFloat();
					const float max = _getReqData(Request::Max).toFloat();
					if(lt.row() == Request::Max) {
						if(min > max) {
							_setReqData(Request::Min, max);
						}
					} else if(lt.row() == Request::Min) {
						if(min > max) {
							_setReqData(Request::Max, min);
						}
					}
				});
	}
	void MainWindow::keepRemoving(const QModelIndex& parent, const int first, const int last) {
		Q_UNUSED(parent)
		auto* m = _keepModel;
		for(int i=first ; i<=last ; i++) {
			const QString path = m->data(m->index(i, 0), Qt::UserRole).toString();
			const auto itr = _keepSet.find(path);
			if(itr != _keepSet.end())
				_keepSet.erase(itr);
			emit keepChanged(path, false);
		}
		emit showLabelFrame(true);
	}
	void MainWindow::_initKeepModel() {
		Q_ASSERT(!_keepModel);
		_keepModel = new QStandardItemModel(this);
		_ui->listKeep->setModel(_keepModel);
		connect(_keepModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
				this, SLOT(keepRemoving(QModelIndex,int,int)));

		// 前回のパスリストを反映
		QSettings s;
		s.beginGroup(c_mainwindow);
		const int n = s.beginReadArray(c_keepmodel);
		for(int i=0 ; i<n ; i++) {
			s.setArrayIndex(i);
			setKeep(s.value("path").toString(), true);
		}
		s.endArray();
	}
	void MainWindow::_emitSprinkleCounterChanged() {
		emit sprinkleCounterChanged(_shown.size(), _notshown.size());
	}
	void MainWindow::sprinkleReset() {
		for(auto& path : _shown)
			_notshown.insert(path);
		_shown.clear();
		_shownP.clear();
		_notshownP = _notshown;
		_emitSprinkleCounterChanged();
	}
	void MainWindow::_initDirModel() {
		Q_ASSERT(!_dirModel);
		_dirModel = new QStandardItemModel(this);
		// モデルに追加、変更(Column1: Path)があった時に通知
		connect(_dirModel, SIGNAL(itemChanged(QStandardItem*)),
				this, SLOT(syncImageSizeList(QStandardItem*)));
		connect(_dirModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
				this, SLOT(dirRemoving(QModelIndex,int,int)));

		// 前回のパスリストを反映
		QSettings s;
		s.beginGroup(c_mainwindow);
		auto* m = _dirModel;
		// パスの復元
		m->clear();
		m->insertColumns(0, 2);
		m->setHeaderData(0, Qt::Horizontal, tr("Path"), Qt::EditRole);
		m->setHeaderData(1, Qt::Horizontal, tr("ImageCount"), Qt::EditRole);

		const int n = s.beginReadArray(c_dirmodel);
		{
			int idx = 0;
			for(int i=0 ; i<n ; i++) {
				s.setArrayIndex(i);
				QString path = s.value("path").toString();
				// 無効なパスを除く
				if(!path.isEmpty()) {
					QDir dir(path);
					if(dir.exists()) {
						m->appendRow(new QStandardItem);
						m->setData(m->index(idx, 0), path, Qt::EditRole);
						++idx;
					}
				}
			}
		}
		s.endArray();
	}
	void MainWindow::_initDirList() {
		Q_ASSERT(!_dirList);
		_dirList = new DirList(_dirModel);
		// ウィンドウ閉じる時の通知
		connect(_dirList, SIGNAL(onClose()), this, SLOT(dirListClosed()));
	}
	void MainWindow::_initWatchList() {
		Q_ASSERT(!_watchList);

		_watcher = new dg::Watcher(this);
		_watchList = new WatchList(_watcher->model());
		_watcher->makeArea(_watchList->getAddArea());
		_watcher->startLoop();

		_qtntf = new dg::QtWNotifier(this);
		_quantizer = new dg::Quantizer(QuantifySize, this);
		// XWindowの移動があったらQuantizerに通知
		connect(_watcher, SIGNAL(onRectChanged(dg::Rect_NameV)), _quantizer, SLOT(onRectChanged(dg::Rect_NameV)));
		// Qtウィンドウの移動があったらQuantizerに通知
		connect(_qtntf, SIGNAL(onQtGeometryChanged()), _quantizer, SLOT(onQtGeometryChanged()));
		// ウィンドウ閉じる時の通知
		connect(_watchList, SIGNAL(onClose()), this, SLOT(watchListClosed()));
		// Quantizerの結果をRectWindowへ伝える
		connect(_quantizer, SIGNAL(onGridChanged(dg::DomainSet, dg::CellBoard, size_t)),
				_ui->rectw, SLOT(onGridChanged(dg::DomainSet, dg::CellBoard, size_t)));
	}
	void MainWindow::watchListClosed() {
		_ui->actionOpenWatchList->setChecked(false);
	}
	void MainWindow::dirListClosed() {
		_ui->actionOpenDirList->setChecked(false);
	}
	void MainWindow::dirRemoving(const QModelIndex& index, const int first, const int last) {
		Q_UNUSED(index);
		auto* m = _dirModel;
		const auto prevSize = _notshown.size();
		for(int i=first ; i<=last ; i++) {
			_removeDirItem(m->item(i,0));
		}
		qDebug() << "dirRemove: " << prevSize << " -> " << _notshown.size();
		_emitSprinkleCounterChanged();
	}
	void MainWindow::_removeDirItem(QStandardItem* item) {
		const QVariant var = item->data(Qt::UserRole);
		if(!var.canConvert<const PathV&>())
			return;

		const auto prevSize = _notshown.size();
		const PathV& pathv = var.value<PathV>();
		for(auto& path : pathv) {
			// shownかnotshown何れかのセットにパスが登録してあるはずなので、削除
			if(auto itr = _shown.find(path);
					itr != _shown.end())
				_shown.erase(itr);
			else if(auto itr = _notshown.find(path);
					itr != _notshown.end())
				_notshown.erase(itr);
			else {
				Q_ASSERT(false);
			}
			auto itr = _imageSet.find(path);
			Q_ASSERT(itr != _imageSet.end());
			_imageSet.erase(itr);
		}
		_shownP = _shown;
		_notshownP = _notshown;
		qDebug() << "_removeDirItem: " << prevSize << " -> " << _notshown.size();
		item->setData(QVariant(), Qt::EditRole);
		item->setData(QVariant(), Qt::UserRole);
	}
	void MainWindow::_CollectImageInDir(PathV& imgv, ImageSet& imgs, const QString& path, const bool recursive) {
		QDir dir(path);
		Q_ASSERT(dir.exists());
		const auto files = dir.entryInfoList(
			{"*.png", "*.jpg", "*.bmp", "*.gif"},
			QDir::Files|QDir::Readable
		);
		for(const QFileInfo& f : files) {
			_CollectImage(imgv, imgs, f.absoluteFilePath());
		}
		if(recursive) {
			// 下層のディレクトリを走査
			const auto dirs = dir.entryInfoList(
				{},
				QDir::Dirs|QDir::Readable|QDir::NoDotAndDotDot
			);
			for(const QFileInfo& f : dirs) {
				if(f.fileName() == "thumbnails")
					continue;
				_CollectImageInDir(imgv, imgs, f.absoluteFilePath(), true);
			}
		}
	}
	void MainWindow::_CollectImage(PathV& imgv, ImageSet& imgs, const QString& path) {
		QImageReader reader(path);
		// 有効な画像だけ読み込む
		if(reader.canRead()) {
			imgv.push_back(path);
			Q_ASSERT(!imgs.contains(path));
			imgs.insert(path, reader.size());
		}
	}
	void MainWindow::syncImageSizeList(QStandardItem* item) {
		qDebug() << "syncImage(Begin): " << _notshown.size();
		const QString path = item->data(Qt::EditRole).toString();
		if(path.isEmpty())
			return;
		// ディレクトリ内の画像を列挙してリスト化
		PathV imgv;
		_CollectImageInDir(imgv, _imageSet, path, true);
		for(auto&& path : imgv) {
			Q_ASSERT(!_notshown.contains(path));
			_notshown.insert(path);
		}

		// ModelにはImageBag-Idの配列(ImageV)を記録しておく
		auto* m = _dirModel;
		m->blockSignals(true);
		{
			const auto row = item->row();
			QStandardItem* item0 = m->itemFromIndex(m->index(row, 0));
			// 古い画像リストはセットから除く
			_removeDirItem(item0);
			// 1列目のUserRoleに画像ファイルパス & サイズのリストを記録
			item0->setData(QVariant::fromValue(imgv), Qt::UserRole);
			// 2列目に画像ファイル数のカウントを表示
			m->setData(m->index(row, 1), int(imgv.size()), Qt::EditRole);
		}
		m->blockSignals(false);
		qDebug() << "syncImage(End): " << _notshown.size();
		_notshownP = _notshown;
		_emitSprinkleCounterChanged();
	}
	void MainWindow::showWatchList(const bool show) {
		if(show)
			_watchList->show();
		else
			_watchList->hide();
	}
	void MainWindow::showDirList(const bool show) {
		if(show)
			_dirList->show();
		else
			_dirList->hide();
	}
	void MainWindow::stayOnTop(const bool top) {
		if(top)
			setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
		else
			setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
		show();
	}
	void MainWindow::_saveDirModel(QSettings& s) {
		s.beginWriteArray(c_dirmodel);
		{
			auto* m = _dirModel;
			const int n = m->rowCount();
			for(int i=0 ; i<n ; i++) {
				s.setArrayIndex(i);
				// 1列目のディレクトリパスのみ記録
				s.setValue("path", m->data(m->index(i, 0), Qt::EditRole));
			}
		}
		s.endArray();
	}
	void MainWindow::_saveKeepModel(QSettings& s) {
		s.beginWriteArray(c_keepmodel);
		{
			auto* m = _keepModel;
			const int n = m->rowCount();
			for(int i=0 ; i<n ; i++) {
				s.setArrayIndex(i);
				const QString path = m->data(m->index(i,0), Qt::UserRole).toString();
				s.setValue("path", path);
			}
		}
		s.endArray();
	}
	void MainWindow::_clearLabels() {
		for(GLabel* lb : _label) {
			lb->close();
			delete lb;
		}
		_label.clear();
		_qtntf->onQtGeometryChanged();
	}
	void MainWindow::_saveInfo() {
		// ウィンドウサイズと、子ウィンドウの表示状態を保存
		QSettings s;
		s.beginGroup(c_mainwindow);
		s.setValue("geometry", saveGeometry());
		s.setValue("showWatchList", _ui->actionOpenWatchList->isChecked());
		s.setValue("showDirList", _ui->actionOpenDirList->isChecked());
		s.setValue("stayOnTop", _ui->actionStayOnTop->isChecked());

		_saveDirModel(s);
		_saveKeepModel(s);
	}
	void MainWindow::closeEvent(QCloseEvent* e) {
		_saveInfo();

		// ---- 後始末 ----
		_clearLabels();
		// 子ウィンドウを全て閉じる
		_watchList->close();
		_dirList->close();

		// Geneスレッド終了 & 待機
		_workerThread->quit();
		_workerThread->wait();

		_toast.reset();
		qApp->setQuitOnLastWindowClosed(true);
		QMainWindow::closeEvent(e);
	}
	void MainWindow::changeEvent(QEvent* e) {
		if(e->type() == QEvent::WindowStateChange) {
			const auto state = windowState();
			if(state == Qt::WindowMinimized) {
				// 最小化と同時に非表示
				hide();
				_actionShow->setChecked(false);
			} else if(state == Qt::WindowNoState) {
				_actionShow->setChecked(true);
			}
		}
	}
	void MainWindow::showEvent(QShowEvent* e) {
		if(!_obstacle) {
			_obstacle = true;
			windowHandle()->setProperty("obstacle", QVariant::fromValue(true));
		}
		QMainWindow::showEvent(e);
	}
	void MainWindow::removeKeep() {
		QModelIndexList sel = _ui->listKeep->selectionModel()->selectedRows();
		std::sort(sel.begin(), sel.end());
		int diff = 0;
		for(auto i : sel) {
			_keepModel->removeRow(i.row() + diff);
			--diff;
		}
	}
	void MainWindow::removeKeepAll() {
		_ui->listKeep->selectAll();
		_ui->actionRemoveKeep->trigger();
	}
}
