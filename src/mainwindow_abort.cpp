#include "mainwindow.hpp"

namespace dg { namespace widget {
	void MainWindow::AbortState::onSprinkleAbort(MainWindow& self) {
		// アイドルステートへ移行
		self._setState(State_U(new IdleState()));
	}
	void MainWindow::AbortState::onSprinkle(MainWindow&) {
		Q_ASSERT(false);
	}
	void MainWindow::AbortState::onStop(MainWindow&) {
		// sprinklerクラスがabortを処理する前に再度ボタンを押すとここに来るが、何もしない
	}
	void MainWindow::AbortState::onSprinkleResult(MainWindow&, const dg::place::ResultV&) {
	}
}}
