#include "watcher.hpp"
#include "dragpos.hpp"
#include "winnotifier.hpp"
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QApplication>
#include <QThread>

namespace dg {
	namespace {
		QRect ToQRect(const RECT& r) {
			return {
				r.left,
				r.top,
				r.right - r.left,
				r.bottom - r.top
			};
		}
	}
	Watcher::Watcher(QObject* parent):
		QObject(parent),
		_model(new QStandardItemModel(this))
	{
		WinNotifier* ntf = new WinNotifier(this);
		// ウィンドウの移動、追加削除などがあったら通知
		connect(ntf, SIGNAL(onEvent()), this, SLOT(_refreshWindowRect()));
		// リスト項目が削除されたら通知
		connect(_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
				 this, SLOT(_removeRow(QModelIndex,int,int)));
	}
	void Watcher::_refreshWindowRect() {
		QThread::currentThread()->msleep(100);
		// シグナルで送る矩形リストを構築
		Rect_NameV rv;
		TCHAR buf[256];
		RECT rect;
		for(auto itr = _wset.begin(); itr != _wset.end();) {
			// 既に無効になったウィンドウはセットから排除
			if(!IsWindow(*itr)) {
				itr = _wset.erase(itr);
			} else {
				GetWindowRect(*itr, &rect);
				GetWindowText(*itr, buf, sizeof(buf)/sizeof(buf[0]));
				rv.emplace_back(
					Rect_Name {
						.rect = ToQRect(rect),
						.name = QString(buf)
					}
				);
				++itr;
			}
		}
		emit onRectChanged(std::move(rv));
		_makeModelFromWSet();
	}
	void Watcher::_removeRow(const QModelIndex idx, const int first, const int last) {
		// 削除されたModelに記載されたウィンドウは、WSetからも削除する
		for(int i=first ; i<=last ; i++) {
			const auto idx2 = _model->index(i, 0, idx);
			const QVariant value = _model->data(idx2, Qt::UserRole);
			Q_ASSERT(value.isValid());
			const HWND hw = value.value<HWND>();
			auto itr = _wset.find(hw);
			if(itr != _wset.end())
				_wset.erase(itr);
		}
		_refreshWindowRect();
	}
	void Watcher::startLoop() {
		// 特に何もしない
	}
	QStandardItemModel* Watcher::model() const noexcept {
		return _model;
	}
	void Watcher::makeArea(QHBoxLayout* addArea) {
		DragPos* dp = new DragPos();
		dp->setText("Drag here");
		dp->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		dp->setFrameShape(QFrame::Panel);
		// Window指定されたらWatcherへ通知
		connect(dp, SIGNAL(dragPosition(QPoint)), this, SLOT(draggedPosition(QPoint)));

		addArea->addWidget(dp);
	}
	bool Watcher::_cursorOnQtWindow() {
		return static_cast<bool>(qApp->topLevelAt(QCursor::pos()));
	}
	void Watcher::_makeModelFromWSet() {
		// 登録済ウィンドウリストを一旦削除
		_model->blockSignals(true);
		_model->removeRows(0, _model->rowCount());
		_model->blockSignals(false);

		_model->insertColumn(0);

		TCHAR buf[256];
		for(auto&& hw : _wset) {
			GetWindowText(hw, buf, sizeof(buf)/sizeof(buf[0]));
			QStandardItem* item = new QStandardItem;
			item->setData(buf, Qt::DisplayRole);
			item->setData(QVariant::fromValue(hw), Qt::UserRole);
			_model->appendRow(item);
		}
	}
	void Watcher::draggedPosition(const QPoint p) {
		// 自プロセスが管理するウィンドウだったら何もしない
		if(_cursorOnQtWindow())
			return;

		POINT pt;
		pt.x = p.x();
		pt.y = p.y();
		HWND hw = WindowFromPoint(pt);
		hw = GetAncestor(hw, GA_ROOT);
		// 既にWSetへ登録済みだったらなにもしない
		if(_wset.count(hw) != 0)
			return;
		_wset.emplace(hw);

		// Modelを更新
		_makeModelFromWSet();
		_refreshWindowRect();
	}
}
