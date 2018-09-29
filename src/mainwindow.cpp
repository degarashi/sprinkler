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

Q_DECLARE_METATYPE(dg::ImageV)
Q_DECLARE_METATYPE(dg::KeepData)
namespace dg {
	namespace {
		QString c_mainwindow("MainWindow"),
				c_dirmodel("DirModel"),
				c_keepmodel("KeepModel");
	}
	void MainWindow::receiveResult(const PlaceV& place) {
		for(auto& p : place) {
			QModelIndex idx;
			{
				auto itr = _path2idx.find(p.path);
				if(itr == _path2idx.end()) {
					auto* item = new QStandardItem;
					item->setData(QFileInfo(p.path).fileName(), Qt::EditRole);
					const KeepData kp {
						.path = p.path,
						.keep = false
					};
					item->setData(QVariant::fromValue(kp), Qt::UserRole);
					_keepModel->appendRow(item);
					idx = _keepModel->index(_keepModel->rowCount()-1, 0);
				} else
					idx = itr.value();
			}
			_label.emplace_back(
				new GLabel(
					p.path,
					p.crop,
					p.offset,
					p.resize,
					idx
				)
			);
			auto itr = _notshown.find(ImageTag{{}, p.path});
			if(itr != _notshown.end()) {
				_shown.emplace(*itr);
				_notshown.erase(itr);
			}
		}
		_path2idx.clear();
		_emitSprinkleCounterChanged();
		_setControlsEnabled(true);
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
						menu.addSection("detail menu");
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
						menu.addSection("simplified menu");
						menu.addAction(_ui->actionSprinkle);
						menu.addAction(_ui->actionRe_Sprinkle);
						menu.exec(QCursor::pos());
						break;
					}
					default: break;
				}
			});
			_tray->show();
		}
	}
	MainWindow::MainWindow(QWidget *const parent):
		QMainWindow(parent),
		_ui(new Ui::MainWindow),
		_watchList(nullptr),
		_watcher(nullptr),
		_dirModel(nullptr),
		_reqModel(nullptr),
		_keepModel(nullptr),
		_dirList(nullptr),
		_tray(nullptr)
	{
		_ui->setupUi(this);

		try {
			_initDirModel();
			_initDirList();
			_initWatchList();
			_initRequestModel();
			_initKeepModel();
			_initSystemTray();

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
			_setReqData(Request::Max, 0.8);
		}
		{
			auto* min = _ui->slMin;
			min->setName(tr("Min"));
			min->refSlider()->setRange(10, 100);
			min->setModel(m, Request::Min);
			_setReqData(Request::Min, 0.4);
		}
		{
			auto* samp = _ui->slSamp;
			samp->setName(tr("Samp"));
			samp->refSlider()->setRange(1, 8);
			samp->setModel(m, Request::Sample);
			_setReqData(Request::Sample, 4);
		}
		_ui->cbPreferOriginal->setModel(m, Request::PreferOriginal);
		_setReqData(Request::PreferOriginal, false);
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
	void MainWindow::_initKeepModel() {
		Q_ASSERT(!_keepModel);
		_keepModel = new QStandardItemModel(this);
		_ui->listKeep->setModel(_keepModel);
		connect(_keepModel, &QStandardItemModel::itemChanged,
				this, [this](QStandardItem* item){
			const int row = item->row();
			const auto kp = item->data(Qt::UserRole).value<KeepData>();
			_ui->listKeep->setRowHidden(row, !kp.keep);
		});
		connect(_keepModel, &QStandardItemModel::rowsInserted,
				this, [this](const QModelIndex&, const int first, const int last){
			for(int i=first ; i<=last ; i++) {
				const auto kp = _keepModel->data(_keepModel->index(i,0), Qt::UserRole).value<KeepData>();
				_ui->listKeep->setRowHidden(i, !kp.keep);
			}
		});

		// 前回のパスリストを反映
		QSettings s;
		s.beginGroup(c_mainwindow);
		const int n = s.beginReadArray(c_keepmodel);
		{
			auto* m = _keepModel;
			for(int i=0 ; i<n ; i++) {
				s.setArrayIndex(i);
				QString path = s.value("path").toString();
				m->appendRow(new QStandardItem);
				QModelIndex idx = m->index(i, 0);
				m->setData(idx, QFileInfo(path).fileName(), Qt::EditRole);
				KeepData kp;
				kp.path = path;
				kp.keep = true;
				m->setData(idx, QVariant::fromValue(kp), Qt::UserRole);

				QImage img(path);
				m->setData(
					idx,
					QPixmap::fromImage(img.scaled({64,64}, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
					Qt::DecorationRole
				);
			}
		}
		s.endArray();
	}
	void MainWindow::_emitSprinkleCounterChanged() {
		emit sprinkleCounterChanged(_shown.size(), _notshown.size());
	}
	void MainWindow::sprinkleReset() {
		for(auto& tag : _shown)
			_notshown.emplace(tag);
		_shown.clear();
		_shownP.clear();
		_notshownP = _notshown;
		_clearKeepList();
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
	void MainWindow::dirRemoving(const QModelIndex index, const int first, const int last) {
		Q_UNUSED(index);
		auto* m = _dirModel;
		const auto prevSize = _notshown.size();
		for(int i=first ; i<=last ; i++) {
			_removeImage(m->item(i,0));
		}
		qDebug() << "dirRemove: " << prevSize << " -> " << _notshown.size();
		_emitSprinkleCounterChanged();
	}
	void MainWindow::_removeImage(QStandardItem* item) {
		const QVariant var = item->data(Qt::UserRole);
		if(!var.canConvert<const ImageV&>())
			return;

		const auto prevSize = _notshown.size();
		const ImageV& img = var.value<ImageV>();
		for(auto& tag : img) {
			// shownかnotshown何れかのセットにパスが登録してあるはずなので、削除
			if(auto itr = _shown.find(tag);
					itr != _shown.end())
				_shown.erase(itr);
			else if(auto itr = _notshown.find(tag);
					itr != _notshown.end())
				_notshown.erase(itr);
			else {
				Q_ASSERT(false);
			}
		}
		_shownP = _shown;
		_notshownP = _notshown;
		qDebug() << "_removeImage: " << prevSize << " -> " << _notshown.size();
		item->setData(QVariant(), Qt::EditRole);
		item->setData(QVariant(), Qt::UserRole);
	}
	void MainWindow::_CollectImageInDir(ImageV& imgv, ImageSet& imgs, const QString& path, const bool recursive) {
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
	void MainWindow::_CollectImage(ImageV& imgv, ImageSet& imgs, const QString& path) {
		QImageReader reader(path);
		// 有効な画像だけ読み込む
		if(reader.canRead()) {
			imgv.emplace_back(ImageTag{reader.size(), path});
			imgs.emplace(imgv.back());
		}
	}
	void MainWindow::syncImageSizeList(QStandardItem* item) {
		qDebug() << "syncImage(Begin): " << _notshown.size();
		const QString path = item->data(Qt::EditRole).toString();
		if(path.isEmpty())
			return;
		// 全部QImageReaderで読み込んでサイズだけ格納

		// ディレクトリ内の画像を列挙してリスト化
		ImageV imgv;
		_CollectImageInDir(imgv, _notshown, path, true);

		// ModelにはImageBag-Idの配列(ImageV)を記録しておく
		auto* m = _dirModel;
		m->blockSignals(true);
		{
			const auto row = item->row();
			QStandardItem* item0 = m->itemFromIndex(m->index(row, 0));
			// 古い画像リストはセットから除く
			_removeImage(item0);
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
		_cleanKeepList();
		s.beginWriteArray(c_keepmodel);
		{
			auto* m = _keepModel;
			const int n = m->rowCount();
			for(int i=0 ; i<n ; i++) {
				s.setArrayIndex(i);
				const KeepData data = m->data(m->index(i,0), Qt::UserRole).value<KeepData>();
				s.setValue("path", data.path);
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
	void MainWindow::closeEvent(QCloseEvent* e) {
		// ウィンドウサイズと、子ウィンドウの表示状態を保存
		QSettings s;
		s.beginGroup(c_mainwindow);
		s.setValue("geometry", saveGeometry());
		s.setValue("showWatchList", _ui->actionOpenWatchList->isChecked());
		s.setValue("showDirList", _ui->actionOpenDirList->isChecked());
		s.setValue("stayOnTop", _ui->actionStayOnTop->isChecked());

		_saveDirModel(s);
		_saveKeepModel(s);

		// 子ウィンドウを全て閉じる
		_clearLabels();
		_watchList->close();
		_dirList->close();

		_workerThread->quit();
		_workerThread->wait();
		QMainWindow::closeEvent(e);
	}
	void MainWindow::removeKeep() {
		QModelIndexList sel = _ui->listKeep->selectionModel()->selectedRows();
		for(auto&& idx : sel) {
			auto kp = _keepModel->data(idx, Qt::UserRole).value<KeepData>();
			kp.keep = false;
			_keepModel->setData(idx, QVariant::fromValue(kp), Qt::UserRole);
		}
	}
	void MainWindow::_cleanKeepList() {
		int nR = _keepModel->rowCount();
		for(int i=0 ; i<nR ; i++) {
			QStandardItem* item = _keepModel->item(i);
			if(!item->data(Qt::UserRole).value<KeepData>().keep) {
				_keepModel->removeRow(i);
				--i;
				--nR;
			}
		}
	}
	void MainWindow::_clearKeepList() {
		_keepModel->removeRows(0, _keepModel->rowCount());
	}
}
