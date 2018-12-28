#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "sprinkler.hpp"
#include "place/result.hpp"
#include "widget/imagelabel.hpp"
#include "toast_mgr.hpp"
#include "dbimage_if.hpp"
#include <QSystemTrayIcon>
#include <QStyle>

namespace dg { namespace widget {
	MainWindow::ProcState::ProcState(TagIdV&& tag):
		_tag(std::move(tag))
	{}
	void MainWindow::ProcState::_setEnable(MainWindow& self, const bool b) {
		self._ui->paramFrame->setEnabled(b);
		self._ui->cbHide->setEnabled(b);
		self._ui->pbSprinkle->setText(b ? "Sprinkle" : "Stop");
	}
	void MainWindow::ProcState::onEnter(MainWindow& self) {
		// システムトレイを処理中アイコンにする
		self._tray->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
		// パラメータ関連のUIを無効にする
		_setEnable(self, false);
		// HideOnSprinkleがOnならウィンドウを隠す
		if(self._ui->cbHide->isChecked()) {
			self._hideSubWindow();
			self.setWindowState(Qt::WindowMinimized);
		}
		// 後はSprinklerクラスに任せる(結果はplaceResultシグナル)
		const auto param = self._ui->request->param();
		Q_ASSERT(param.sizeRange.from <= param.sizeRange.to);
		sprinkler.sprinkle(param, _tag);
	}
	void MainWindow::ProcState::onExit(MainWindow& self) {
		// パラメータ関連のUIを元に戻す
		_setEnable(self, true);
		// システムトレイを通常アイコンにする
		self._tray->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarMenuButton));
	}
	void MainWindow::ProcState::onSprinkle(MainWindow&) {
		// 中止要請
	}
	void MainWindow::ProcState::onSprinkleResult(MainWindow& self, const dg::place::ResultV& res) {
		QString title,
				msg;
		if(res.empty()) {
			title = tr("No image");
			msg = tr("There's no image can place");
		} else {
			title = tr("Image placed");
			msg = tr("%n image(s) placed", "", res.size());
			for(auto& r : res) {
				auto* lb = new widget::ImageLabel(
					r.id,
					r.crop,
					r.offset,
					r.resize,
					self._ctrlMenu,
					self._dbImg,
					self._dbTag
				);
				// どれか1つをクリックしたら他の全てのGLabelと自分を前面に持ってくる
				connect(lb, &widget::ImageLabel::clicked,
					&self, [&self](){
						for(auto* l : self._label)
							l->raise();
						emit self.labelClicked();
					});
				self._label.emplace_back(lb);
			}
		}
		mgr_toast.bake(
			Toast::Icon::Information,
			title,
			msg
		);
		self._setState(State_U(new IdleState()));
	}
}}
