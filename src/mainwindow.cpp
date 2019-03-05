#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "sprinkler.hpp"
#include "dbimage_if.hpp"
#include "dbtag_if.hpp"
#include "toast_mgr.hpp"
#include "version.hpp"
#include "widget/imagelabel.hpp"
#include "database_sig.hpp"
#include "place/result.hpp"
#include <QSystemTrayIcon>
#include <QStyle>
#include <QShowEvent>
#include <QHideEvent>

namespace dg { namespace widget {
	namespace {
		QString c_mainwindow("MainWindow"),
				c_watchlist("watch"),
				c_rect("rect"),
				c_source("source"),
				c_tag("tag");
	}
	void MainWindow::_clearLabels() {
		for(auto* lb : _label) {
			lb->close();
			delete lb;
		}
		_label.clear();
	}
	void MainWindow::sprinkleProgress(const int percent) {
		// 進捗をプログレスバーに反映
		_ui->progress->setValue(percent);
	}
	void MainWindow::_refresh_counter_tag(const TagIdV& tag) {
		const auto c = _dbTag->countImageByTag(tag);
		emit remainingImageCounterChanged(tag, c.total, c.shown);
	}
	void MainWindow::_refresh_counter() {
		_refresh_counter_tag(_ui->tagSelector->getArray());
	}
	MainWindow::MainWindow(DBImage* img, DBTag* tag, DatabaseSignal* sig, QWidget *parent) :
		base_t("placer", parent),
		_ui(new Ui::MainWindow),
		_dbTag(tag),
		_dbImg(img)
	{
		_ui->setupUi(this);
		{
			// バージョン番号の表示
			const auto ver = QString("Sprinkler (v%1)").arg(QString::fromStdString(Version::ThisVersion().asString()));
			setWindowTitle(ver);
		}
		_ui->tagSelector->init(tag, sig);

		// 画像コレクションが変わる時には既に配置してあるラベルと、前回の画像リストをクリア
		connect(sig, &DatabaseSignal::beginResetDir,
			this, [this](){
				_clearLabels();
				_prevImg.clear();
			});

		auto* spr = &sprinkler;
		// --------- sprinkle コネクション ---------
		// sprinkle経過
		connect(spr, &Sprinkler::sprinkleProgress,
				this, &MainWindow::sprinkleProgress);

		// sprinkle結果
		connect(spr, &Sprinkler::sprinkleResult,
					this, [this](const place::ResultV& r){
						_state->onSprinkleResult(*this, r); });
		connect(spr, &Sprinkler::sprinkleAbort,
					this, [this](){
						_state->onSprinkleAbort(*this); });
		connect(_ui->tagSelector, &TagSelector::changed,
					this, &MainWindow::_refresh_counter_tag);
		connect(spr, &Sprinkler::imageChanged,
					this, &MainWindow::_refresh_counter);

		// --------- ツールバー ---------
		_ui->toolBar->addAction(spr->getAction(Sprinkler::Action::OpenDir));
		_ui->toolBar->addAction(spr->getAction(Sprinkler::Action::OpenTag));
		_ui->toolBar->addAction(spr->getAction(Sprinkler::Action::OpenRect));
		_ui->toolBar->addAction(spr->getAction(Sprinkler::Action::OpenWatch));
		_ui->toolBar->addSeparator();
		_ui->toolBar->addAction(_ui->actionStayOnTop);

		_ui->actionmenu->addAction(spr->getAction(Sprinkler::Action::ResetFlag));
		_ui->appmenu->addAction(spr->getAction(Sprinkler::Action::Quit));

		_ui->windowmenu->addAction(spr->getAction(Sprinkler::Action::OpenDir));
		_ui->windowmenu->addAction(spr->getAction(Sprinkler::Action::OpenTag));
		_ui->windowmenu->addAction(spr->getAction(Sprinkler::Action::OpenRect));
		_ui->windowmenu->addAction(spr->getAction(Sprinkler::Action::OpenWatch));

		// --------- イメージラベルをCtrl + 右クリックした時に表示されるメニュー  ---------
		_ctrlMenu = new QMenu(this);
		_ctrlMenu->addAction(_ui->actionSprinkle);
		_ctrlMenu->addAction(_ui->actionReposition);
		_ctrlMenu->addSeparator();
		_ctrlMenu->addAction(spr->getAction(Sprinkler::Action::Quit));

		// --------- システムトレイ ---------
		if(QSystemTrayIcon::isSystemTrayAvailable()) {
			_tray = new QSystemTrayIcon(this);
			_tray->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarMenuButton));
			connect(_tray, &QSystemTrayIcon::activated,
				this, [this](const QSystemTrayIcon::ActivationReason reason){
					auto* spr = &sprinkler;
					switch(reason) {
						case QSystemTrayIcon::Context:
						{
							// 詳細メニュー
							QMenu menu(this);
							menu.addSection(tr("detail menu"));
							menu.addAction(spr->getAction(Sprinkler::Action::OpenMain));
							menu.addSeparator();
							// ウィンドウ開く系のアクション
							menu.addAction(spr->getAction(Sprinkler::Action::OpenDir));
							menu.addAction(spr->getAction(Sprinkler::Action::OpenTag));
							menu.addAction(spr->getAction(Sprinkler::Action::OpenRect));
							menu.addAction(spr->getAction(Sprinkler::Action::OpenWatch));
							menu.addSeparator();
							menu.addAction(_ui->actionSprinkle);
							menu.addAction(_ui->actionReposition);
							menu.addSeparator();
							menu.addAction(spr->getAction(Sprinkler::Action::ResetFlag));
							menu.addSeparator();
							menu.addAction(spr->getAction(Sprinkler::Action::Quit));
							menu.exec(QCursor::pos());
							break;
						}
						case QSystemTrayIcon::Trigger:
						{
							// 簡易メニュー
							QMenu menu(this);
							menu.addSection(tr("simplified menu"));
							menu.addAction(spr->getAction(Sprinkler::Action::OpenMain));
							menu.addSeparator();
							menu.addAction(_ui->actionSprinkle);
							menu.addAction(_ui->actionReposition);
							menu.exec(QCursor::pos());
							break;
						}
						default: break;
					}
			});
			_tray->show();
			// タスクトレイをホバーすると対象タグと
			// 今どの程度のイメージが表示されたのかの割合を表示
			connect(this, &MainWindow::remainingImageCounterChanged,
					this, [this](const TagIdV& tag, const size_t total, const size_t shown) {
					QStringList sl;
					for(auto id : tag) {
						sl.append(_dbTag->getTagName(id));
					}
					_tray->setToolTip(QString(tr("showing: %1\n[%2 / %3] were displayed"))
						.arg(sl.join(','))
						.arg(shown)
						.arg(total));
			});
		}
		_setState(State_U{new IdleState});
	}
	void MainWindow::_setState(State_U state) {
		if(_state)
			_state->onExit(*this);
		std::swap(_state, state);
		if(_state)
			_state->onEnter(*this);
	}
	void MainWindow::_hideSubWindow() {
		const auto hide_f = [](const auto f){
			sprinkler.getAction(f)->setChecked(false);
		};
		hide_f(Sprinkler::Action::OpenDir);
		hide_f(Sprinkler::Action::OpenTag);
		hide_f(Sprinkler::Action::OpenRect);
		hide_f(Sprinkler::Action::OpenWatch);
	}
	void MainWindow::closeEvent(QCloseEvent* e) {
		// サブウィンドウの表示状態をセーブ, そして閉じる
		QSettings s;
		s.beginGroup(c_mainwindow);
		const auto save = [&s](const QString& name, const auto num){
			s.setValue(name, sprinkler.getAction(num)->isChecked());
		};
		save(c_watchlist, Sprinkler::Action::OpenWatch);
		save(c_rect, Sprinkler::Action::OpenRect);
		save(c_source, Sprinkler::Action::OpenDir);
		save(c_tag, Sprinkler::Action::OpenTag);

		_hideSubWindow();

		base_t::closeEvent(e);
	}
	void MainWindow::stayOnTop(const bool b) {
		if(b)
			setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
		else
			setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
		show();
	}
	void MainWindow::sprinkle() {
		_state->onSprinkle(*this);
	}
	void MainWindow::stop() {
		_state->onStop(*this);
	}
	void MainWindow::reposition() {
		_state->onReposition(*this);
	}
	void MainWindow::resetViewFlagSelecting() {
		// 選択中のタグを取得
		const auto tag = _ui->tagSelector->getArray();
		// 何も選択されてなければリセット処理もしない
		if(tag.empty())
			return;
		_dbImg->resetViewFlagSelected(tag);
	}
	void MainWindow::resetViewFlagAll() {
		_dbImg->resetViewFlag();
	}
	void MainWindow::changeEvent(QEvent* e) {
		if(e->type() == QEvent::WindowStateChange) {
			const auto state = windowState();
			if(state == Qt::WindowMinimized) {
				// 最小化と同時に非表示
				hide();
			} else if(state == Qt::WindowNoState) {
				show();
			}
		}
	}
}}
