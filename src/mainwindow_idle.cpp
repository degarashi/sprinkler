#include "mainwindow.hpp"
#include "dbtag_if.hpp"
#include "ui_mainwindow.h"
#include "toast_mgr.hpp"
#include <QTimer>
#include <QDebug>

namespace dg { namespace widget {
	void MainWindow::IdleState::onSprinkle(MainWindow& self) {
		self._clearLabels();

		TagIdV tag = self._ui->tagSelector->getArray();
		const auto c = self._dbTag->countImageByTag(tag);
		// 対象画像数が0だったら何もしない
		if(c.notshown() == 0) {
			mgr_toast.bake(
				Toast::Icon::Information,
				tr("No images"),
				tr("There's no image to show.\n(Image-set runs out)")
			);
			return;
		}
		self._setState(State_U{new ProcState(std::move(tag), true)});
	}
	void MainWindow::IdleState::onStop(MainWindow&) {
		Q_ASSERT(false);
	}
	void MainWindow::IdleState::onReposition(MainWindow& self) {
		self._clearLabels();

		// 対象画像数が0だったら何もしない
		if(self._prevImg.empty()) {
			mgr_toast.bake(
				Toast::Icon::Information,
				tr("No images"),
				tr("There's no image to show.\n(No previous image set)")
			);
			return;
		}
		self._setState(State_U{new ProcState(self._prevImg, false)});
	}
	void MainWindow::IdleState::onSprinkleResult(MainWindow&, const dg::place::ResultV&) {
		Q_ASSERT(false);
	}
	void MainWindow::IdleState::onSprinkleAbort(MainWindow&) {
		Q_ASSERT(false);
	}
}}
