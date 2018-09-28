#include <QDebug>
#include <unistd.h>
#include <QApplication>
#include <QStandardItemModel>
#include "watcher.hpp"
#include "clientwin.h"
#include <QHBoxLayout>
#include <QPushButton>

namespace dg {
	bool g_Xerror;
	int HandleError(_XDisplay*, XErrorEvent* err) {
		qDebug() << "ERROR: X11 error: code=" << err->error_code;
		g_Xerror = true;
		return 1;
	}

	void Watcher::WatchEntry::invalidateRect() noexcept {
		rect = QRect(0,0,0,0);
	}
	bool Watcher::WatchEntry::isEmpty() const noexcept {
		return rect.isEmpty();
	}

	Watcher::Watcher(QObject* parent):
		QObject(parent),
		_model(new QStandardItemModel(this))
	{
		connect(_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
				 this, SLOT(_removeRow(QModelIndex,int,int)));
	}
	void Watcher::makeArea(QHBoxLayout* addArea) {
		QPushButton* pb = new QPushButton();
		pb->setText(tr("Add"));
		pb->setIcon(QIcon::fromTheme("list-add"));
		addArea->addWidget(pb);

		// GUIからXWindow追加の指示があったらwatchスレッドへ通知
		connect(pb, SIGNAL(clicked()), this, SLOT(addWatch()));
	}
	void Watcher::_removeRow(const QModelIndex idx, const int first, const int last) {
		for(int i=first ; i<=last ; i++) {
			const auto idx2 = _model->index(i, 0, idx);
			const QVariant value = _model->data(idx2, Qt::UserRole);
			Q_ASSERT(value.isValid());
			const int wid = value.toInt();
			_removeWatch(static_cast<dg::Watcher::WatchId>(wid));
		}
	}
	Watcher::~Watcher() {
		if(_thread.joinable()) {
			// スレッドへ終了通知を送る
			const Command cmd = {
				.type = Command::Quit
			};
			write(_cmdFd, &cmd, sizeof(cmd));

			_thread.join();
			close(_cmdFd);
		}
	}
	uint32_t Watcher::_EventMask() {
		return StructureNotifyMask | VisibilityChangeMask;
	}
	void Watcher::_ThreadProc(Watcher* self, const int cmdFd) noexcept {
		const bool bError = [=](){
			try {
				Display_U disp_u(XOpenDisplay(nullptr));
				if(!disp_u) {
					throw std::runtime_error(tr("unable to connect to display").toStdString());
				}
				XSetErrorHandler(HandleError);

				auto* disp = disp_u.get();
				const int x11_fd = ConnectionNumber(disp);
				const int maxfd = std::max(x11_fd, cmdFd) + 1;
				const Window root = DefaultRootWindow(disp);
				const auto mask = _EventMask();
				const auto getwindowrect = [disp, root](const Window w){
					Window child;
					int x, y;
					XWindowAttributes attr;
					XGetWindowAttributes(disp, w, &attr);
					XTranslateCoordinates(disp, w, root, 0, 0, &x, &y, &child);
					return QRect {
						x - attr.x,
						y - attr.y,
						attr.width,
						attr.height
					};
				};
				for(;;) {
					fd_set fds;
					FD_ZERO(&fds);
					FD_SET(x11_fd, &fds);
					FD_SET(cmdFd, &fds);
					timeval tv {
						.tv_sec = 1,
						.tv_usec = 0
					};
					XFlush(disp);
					const int ret = select(maxfd, &fds, nullptr, nullptr, &tv);
					if(ret > 0) {
						if(FD_ISSET(x11_fd, &fds)) {
							bool stateChanged = false;
							// XWindow イベント受信
							while(XPending(disp)) {
								XEvent e;
								XNextEvent(disp, &e);

								const auto modent = [self, &stateChanged](const auto& s, auto&& cb){
									std::lock_guard lk(self->_mutex);
									if(auto* ent = self->_findEntry(WindowD(s.window))) {
										cb(s, ent);
										stateChanged = true;
									}
								};
								auto dbg = qDebug() << "(WatchThread): ";
								switch(e.type) {
									case ConfigureNotify:
										modent(e.xconfigure, [&](const auto& info, auto* ent){
											dbg << "WindowMoved: " << info.window << QRect(info.x, info.y, info.width, info.height) << ", " << info.serial;
											ent->rect = getwindowrect(ent->window.template as<Window>());
										});
										break;
									case DestroyNotify:
										{
											const Window w = e.xdestroywindow.window;
											dbg << "WindowDestroyed: " << w;

											// 監視ウィンドウリストから対象ウィンドウを外す
											std::lock_guard lk(self->_mutex);
											if(self->_removeEntry(w))
												stateChanged = true;
										}
										break;
									case UnmapNotify:
										modent(e.xunmap, [&dbg](const auto& info, auto* ent){
											dbg << "Unmap: " << info.window;
											ent->invalidateRect();
										});
										break;
									case VisibilityNotify:
										modent(e.xvisibility, [disp, &dbg, &getwindowrect](const auto& info, auto* ent){
											if(info.state == VisibilityFullyObscured) {
												// 他のウィンドウに完全に覆われた場合はサイズを0にする
												ent->invalidateRect();
												dbg << "Obscured : " << info.window;
											} else {
												dbg << ((info.state==VisibilityPartiallyObscured)? "Partially: " : "UnObscured : ") << info.window;
												// サイズを取得し直す
												ent->rect = getwindowrect(ent->window.template as<Window>());
											}
										});
										break;
								}
							}
							if(stateChanged)
								QMetaObject::invokeMethod(self, "_onStateChanged", Qt::QueuedConnection);
						} else if(FD_ISSET(cmdFd, &fds)) {
							// スレッドに対するコマンド
							Command cmd;
							read(cmdFd, &cmd, sizeof(cmd));
							auto dbg = qDebug() << "(WatchThread): ";
							switch(cmd.type) {
								case Command::AddWatch:
									dbg << "AddWatch";
									{
										Window target = SelectWindow(disp, root).as<Window>();
										target = Find_Client(disp, root, target);
										XSelectInput(disp, target, mask);
										const auto name = GetWindowName(disp, target);
										{
											XWindowAttributes attr;
											XGetWindowAttributes(disp, target, &attr);
											std::lock_guard lk(self->_mutex);
											// 既に登録済みのウィンドウなら何もしない
											if(!self->_findEntry(WindowD(target))) {
												// 自アプリケーションのウィンドウなら何もしない
												bool qtwin;
												{
													const bool ret = QMetaObject::invokeMethod(self,
																		"_cursorOnQtWindow", Qt::BlockingQueuedConnection,
																		Q_RETURN_ARG(bool, qtwin));
													Q_ASSERT(ret);
												}
												if(!qtwin) {
													self->_entry.emplace_back(WatchEntry{
														.id = self->_serialId++,
														.window = target,
														.name = name ? *name : tr("(noname)").toStdString(),
														.rect = getwindowrect(target)
													});
													QMetaObject::invokeMethod(self, "_onStateChanged", Qt::QueuedConnection);
												}
											}
										}
									}
									break;
								case Command::RemoveWatch:
									dbg << "RemoveWatch";
									XSelectInput(disp, cmd.removewatch.window.as<Window>(), 0);
									break;
								case Command::Quit:
									dbg << "Quit";
									return false;
							}
						}
					} else if(ret==0) {
						// timeout
					} else {
						throw std::runtime_error(tr("error in select()").toStdString());
					}
				}
				return false;
			} catch(const std::exception& e) {
				qDebug() << "(WatchThread): Error in Watcher::Thread: " << e.what();
				return true;
			}
		}();
		QMetaObject::invokeMethod(self, "_onExit", Qt::QueuedConnection, Q_ARG(bool, bError));
		close(cmdFd);
	}
	void Watcher::_onStateChanged() {
		std::lock_guard lk(_mutex);
		// 登録済ウィンドウリストを一旦削除
		_model->blockSignals(true);
		_model->removeRows(0, _model->rowCount());
		_model->blockSignals(false);

		_model->insertRows(0, _entry.size());
		_model->insertColumns(0, _entry.size());

		// シグナルで送る矩形リストを構築
		Rect_NameV rect;
		const auto nE = _entry.size();
		for(size_t i=0 ; i<nE ; i++) {
			auto& e = _entry[i];
			if(!e.rect.isEmpty()) {
				rect.emplace_back(
					Rect_Name {
						.rect = e.rect,
						.name = QString(e.name.c_str())
					}
				);
			}

			const QModelIndex index(_model->index(i, 0));
			_model->setData(index, QString::fromStdString(e.name), Qt::DisplayRole);
			_model->setData(index, e.id, Qt::UserRole);
		}
		qDebug() << "Watcher::_onStateChanged()" << _entry.size();
		emit onRectChanged(std::move(rect));
	}
	void Watcher::_onExit(const bool bError) {
		if(bError)
			qDebug() << "Watcher::_onExit(): Error";
	}
	void Watcher::_removeWatch(const WatchId id) {
		if(!_thread.joinable())
			throw std::runtime_error(tr("Not running thread").toStdString());

		WindowD target;
		{
			std::lock_guard lk(_mutex);
			if(auto* ent = _findEntry(id))
				target = ent->window;
			else
				return;
			_removeEntry(target);
		}
		const Command cmd = {
			.type = Command::RemoveWatch,
			.removewatch = {
				.window = target
			}
		};
		write(_cmdFd, &cmd, sizeof(cmd));
	}
	void Watcher::addWatch() {
		if(!_thread.joinable())
			throw std::runtime_error(tr("Not running thread").toStdString());

		const Command cmd = {
			.type = Command::AddWatch
		};
		write(_cmdFd, &cmd, sizeof(cmd));
	}
	void Watcher::startLoop() {
		if(_thread.joinable())
			throw std::runtime_error(tr("Already running").toStdString());

		int cmd[2];
		pipe(cmd);
		_cmdFd = cmd[Write];
		_thread = std::thread(_ThreadProc, this, cmd[Read]);
	}
	bool Watcher::_removeEntry(const WindowD wd) {
		const auto itr = std::find_if(_entry.begin(), _entry.end(), [wd](const auto& e){
			return e.window == wd;
		});
		if(itr != _entry.end()) {
			_entry.erase(itr);
			QMetaObject::invokeMethod(this, "_onStateChanged", Qt::QueuedConnection);
			return true;
		}
		return false;
	}
	Watcher::WatchEntry* Watcher::_findEntry(const WatchId id) {
		const auto itr = std::find_if(_entry.begin(), _entry.end(), [id](const auto& e){
			return e.id == id;
		});
		if(itr == _entry.end())
			return nullptr;
		return &(*itr);
	}
	Watcher::WatchEntry* Watcher::_findEntry(const WindowD wd) {
		const auto itr = std::find_if(_entry.begin(), _entry.end(), [wd](const auto& e){
			return e.window == wd;
		});
		if(itr == _entry.end())
			return nullptr;
		return &(*itr);
	}
	QStandardItemModel* Watcher::model() const noexcept {
		return _model;
	}
	bool Watcher::_cursorOnQtWindow() {
		return static_cast<bool>(qApp->topLevelAt(QCursor::pos()));
	}
}
