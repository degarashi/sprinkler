#include "sprinkler.hpp"
#include "toast_mgr.hpp"
#include "quantizer.hpp"
#include "rectwindow.hpp"
#include "watchlist.hpp"
#include "watcher.hpp"
#include "qtw_notifier.hpp"
#include "imagedir_window.hpp"
#include "imagetag_window.hpp"

#include "database.hpp"
#include "mainwindow.hpp"
#include "sprinkler_aux.hpp"
#include "place/param.hpp"
#include "place/result.hpp"
#include "place/selected.hpp"
#include "lubee/src/low_discrepancy.hpp"
#include "gene_worker.hpp"
#include "taginput.hpp"
#include "sql/transaction.hpp"
#include "toml_settings.hpp"

#include <QApplication>
#include <QAction>
#include <random>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QMenu>

namespace dg {
	Sprinkler::Sprinkler():
		_toast(new ToastMgr(this)),
		_watcher(nullptr),
		_quantizer(nullptr),
		_qtntf(nullptr),
		_state(State::Idle),
		_window{}
	{
		qApp->setQuitOnLastWindowClosed(false);
		_storeConstValues();
		_initAction();
		_initWatchList();
		_initRectView();
		_initImageSrc();
		_linkAction();

		_workerThread = new QThread(this);
		_workerThread->start();
		_geneWorker = new GeneWorker;
		_geneWorker->moveToThread(_workerThread);

		qRegisterMetaType<dg::place::ResultV>("dg::place::ResultV");
		connect(_geneWorker, &GeneWorker::sprinkleResult,
				this, [this](const dg::place::ResultV& r){
					// 実際に使用した画像にフラグを立てる
					{
						ImageIdV used;
						for(auto& img : r)
							used.emplace_back(img.id);
						_db->setViewFlag(used, 2);
					}
					_resetToIdleState(State::Processing);
					emit sprinkleResult(r);
				});
		connect(_geneWorker, &GeneWorker::sprinkleAbort,
				this, [this](){
					_resetToIdleState(State::Aborted);
					emit sprinkleAbort();
				});
		connect(_geneWorker, &GeneWorker::sprinkleProgress,
				this, &Sprinkler::sprinkleProgress);
		connect(_db, &Database::endResetImage,
				this, &Sprinkler::imageChanged);
		getAction(Action::OpenMain)->toggle();
	}
	void Sprinkler::_resetToIdleState(const State::e expected) {
		emit sprinkleProgress(100);
		Q_ASSERT(_state == expected);
		_state = State::Idle;

		// 候補には挙がったが使用されなかった画像のフラグをリセット
		_db->resetSelectionFlag();
	}
	void Sprinkler::_linkAction() {
		auto& a = _action;
		connect(a[Action::OpenDir], &QAction::toggled,
				_window.source, &QWidget::setVisible);
		connect(a[Action::OpenTag], &QAction::toggled,
				_window.tag, &QWidget::setVisible);
		connect(a[Action::OpenRect], &QAction::toggled,
				_window.rect, &QWidget::setVisible);
		connect(a[Action::OpenWatch], &QAction::toggled,
				_window.watchList, &QWidget::setVisible);
		connect(a[Action::OpenMain], &QAction::toggled,
				this, [this](const bool b){
					_window.mainwin->setVisible(b);
					if(b)
						_window.mainwin->setWindowState(Qt::WindowState::WindowNoState);
				});
		connect(a[Action::Quit], &QAction::triggered,
				this, [this](){
					// 全てのウィンドウを閉じた後、アプリケーションを終了
					_window.watchList->close();
					_window.rect->close();
					_window.source->close();
					_window.tag->close();
					_window.mainwin->close();
					qApp->quit();
				});
		connect(a[Action::ResetFlag], &QAction::triggered,
				this, [this](){ _db->resetViewFlag(); });
	}
	void Sprinkler::_initAction() {
		const auto init = [this](const bool checkable, const QString& text, const QString& theme, const QString& seq){
			QIcon icon;
			if(QIcon::hasThemeIcon(theme))
				icon = QIcon::fromTheme(theme);
			else
				icon.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
			auto* a = new QAction(icon, text, this);
			a->setCheckable(checkable);
			a->setShortcut(QKeySequence(seq));
			return a;
		};
		auto& a = _action;
		a[Action::OpenDir] = init(true, tr("OpenImageSource&Dir"), "folder-drag-accept", "Ctrl+D");
		a[Action::OpenTag] = init(true, tr("Open&Tag"), "edit-find", "Ctrl+T");
		a[Action::OpenRect] = init(true, tr("Open&RectView"),  "image", "Ctrl+R");
		a[Action::OpenWatch] = init(true, tr("Open&Watch"), "appointment-new", "Ctrl+W");
		a[Action::OpenMain] = init(true, tr("Open&MainWindow"), {}, "Ctrl+M");
		a[Action::Quit] = init(false, tr("&Quit"), "application-exit", "Ctrl+Q");
		a[Action::ResetFlag] = init(false, tr("&ResetViewFlag"), {}, {});
	}
	QAction* Sprinkler::getAction(const Action::e a) const {
		return _action[a];
	}
	Sprinkler::~Sprinkler() {
		// Geneスレッド終了 & 待機
		_workerThread->quit();
		_workerThread->wait();
		// DatabaseよりQSqlQueryを先に破棄する
		delete _window.tag;
		delete _window.watchList;
		delete _window.rect;
		delete _window.source;
		delete _window.mainwin;
	}
	namespace {
		#define DefP(name, ent, type) \
			Define_TomlSet("Sprinkler", name, ent, type)
		namespace param {
			DefP(AspMin, "Asp.min", float)
			DefP(AspMax, "Asp.max", float)
			DefP(AspDiff, "Asp.diff", float)
			DefP(AreaRatio, "Collect.area_ratio", int)
			DefP(EnumBuckets, "Collect.enum_buckets", size_t)
			DefP(EnumNImage, "Collect.enum_nimage", size_t)
			DefP(AuxImage, "Collect.aux_image", size_t)
		}
		#undef DefP
	}
	void Sprinkler::_storeConstValues() {
		_const.quantify_size = tomlset.toValue<size_t>("Sprinkler.quantify_size");
		_const.delay_ms = tomlset.toValue<size_t>("Sprinkler.delay_ms");
	}
	void Sprinkler::_initRectView() {
		Q_ASSERT(_quantizer);
		Q_ASSERT(!_window.rect);

		_window.rect = new RectWindow();
		// Quantizerの結果をRectViewへ伝える
		connect(_quantizer, &Quantizer::onGridChanged,
				_window.rect, &RectWindow::onGridChanged);
		connect(_window.rect, &RectWindow::onVisibilityChanged,
				_action[Action::OpenRect], &QAction::setChecked);
	}
	void Sprinkler::_initWatchList() {
		Q_ASSERT(!_watcher);

		_watcher = new Watcher(this);
		_window.watchList = new WatchList(_watcher->model());
		connect(_window.watchList, &RectWindow::onVisibilityChanged,
				_action[Action::OpenWatch], &QAction::setChecked);

		_watcher->makeArea(_window.watchList->getAddArea());
		_watcher->startLoop();

		_quantizer = new Quantizer(_const.quantify_size, this);
		_qtntf = new QtWNotifier(_const.delay_ms, this);

		// 監視対象ウィンドウの移動があったらQuantizerに通知
		connect(_watcher, &Watcher::onWatchedRectChanged,
				_quantizer, &Quantizer::onWatchedRectChanged);
		// Qtウィンドウの移動があったらQuantizerに通知
		connect(_qtntf, &QtWNotifier::onQtGeometryChanged,
				_quantizer, &Quantizer::onQtGeometryChanged);
	}
	void Sprinkler::_initImageSrc() {
		Q_ASSERT(!_window.source);
		// auto* db = new Database(Database::TagInit, this);
		auto* db = new Database(this);
		_db = db;
		{
			auto* w = new widget::MainWindow(db, db, db);
			_window.mainwin = w;
			connect(w, &widget::MainWindow::onVisibilityChanged,
					_action[Action::OpenMain], &QAction::setChecked);
		}
		{
			auto* w = new ImageDirWindow(db, db->makeDirModel());
			_window.source = w;
			connect(w, &ImageDirWindow::onVisibilityChanged,
					_action[Action::OpenDir], &QAction::setChecked);
		}
		{
			auto* w = new ImageTagWindow(db, db, db);
			connect(this, &Sprinkler::destroyed,
					w, [w](auto*){ delete w; });
			_window.tag = w;
			connect(w, &ImageTagWindow::onVisibilityChanged,
					_action[Action::OpenTag], &QAction::setChecked);
		}
	}
	namespace {
		QSize ToQSize(const lubee::SizeI& s) noexcept {
			return {s.width, s.height};
		}
	}
	void Sprinkler::_sprinkle(const place::Param& param, const TagIdV& tag) {
		if(_state == State::Aborted) {
			_resetToIdleState(State::Aborted);
			// まだGeneWorkerスレッドに伝えてないのでここで中断シグナルを出す
			emit sprinkleAbort();
			return;
		}
		Q_ASSERT(_state == State::WaitDelay);
		_state = State::Processing;

		sql::Transaction(
			QSqlDatabase::database(),
			[this, &param, &tag](){
				// 最初に候補フラグをクリア
				_db->resetSelectionFlag();

				const CellBoard& board = _quantizer->qmap();
				const auto qs = _const.quantify_size;
				// アスペクト比とサイズの目安(Quantized)
				const auto asp = CalcAspSize(
									board,
									param::AspMin(),
									param::AspMax(),
									param::AspDiff()
								);
				const auto nAsp = asp.size();
				assert(nAsp > 0);
				using Area_t = int_fast32_t;
				// ループを回す残り面積
				Area_t remain(Area_t(board.getNEmptyCell()) * param::AreaRatio());
				// 最後に選択した矩形(quantized) 一枚しか候補となる画像が無かった場合に使用
				lubee::RectI		lastRect;
				// 候補画像リスト
				place::SelectedV	selected;
				ImageIdV			used;
				// 選択した画像をマーク & 重複チェック
				const auto markUsed = [this, &used](){
					const size_t s0 = used.size();
					std::sort(used.begin(), used.end());
					used.erase(std::unique(used.begin(), used.end()), used.end());
					Q_ASSERT(s0 == used.size());
					// 候補フラグを立てる
					_db->setViewFlag(used, 1);
					used.clear();
				};
				// 場の最大サイズ(assert用)
				#ifndef NDEBUG
					const auto maxCellSize = board.nboard().getSize();
				#endif
				for(;;) {
					// アスペクト比均等で画像候補を列挙
					const auto cand = _db->enumImageByAspect(tag, param::EnumBuckets(), param::EnumNImage());
					const auto nCand = cand.size();
					if(nCand == 0) {
						// もう候補が存在しない
						break;
					}
					// 各アスペクト領域から均等に一つずつ、倍率をかけながら候補に加えていって面積から引く
					size_t i=0;
					while(i < nCand) {
						auto& c = cand[i];
						// 画像の元サイズ
						const QSize orig = c.size;
						const float a = float(orig.width()) / orig.height();
						// アスペクト比を維持したまま拡大縮小して少くとも単体で画面に配置できる目安サイズを検索
						lastRect = asp.back().rect;
						lubee::SizeI maxsize = lastRect.size() * qs;
						for(size_t i=0 ; i<nAsp ; i++) {
							auto& a0 = asp[i];
							if(a <= a0.aspect) {
								lastRect = a0.rect;
								maxsize = a0.rect.size() * qs;
								break;
							}
						}
						float r = 1;
						if(orig.width() > maxsize.width) {
							// 横幅をmaxsizeに合わせるような倍率
							r = maxsize.width / float(orig.width());
						}
						if(orig.height() > maxsize.height) {
							// 縦幅をmaxsizeに合わせるような倍率
							r = std::min(r, maxsize.height / float(orig.height()));
						} else {
							// 拡大する
							r = std::min(
								maxsize.width / float(orig.width()),
								maxsize.height / float(orig.height())
							);
						}
						// 画像を空き領域に配置できる限界まで拡大or縮小したサイズ
						const auto fit_w = static_cast<size_t>(std::ceil(r * orig.width())),
									fit_h = static_cast<size_t>(std::ceil(r * orig.height()));
						// 遺伝子候補に加える
						selected.emplace_back(
							place::Selected {
								.id = ImageId(c.id),
								.size = {int32_t(fit_w), int32_t(fit_h)},
							}
						);
						const auto fitQs = selected.back().getQuantizedSize(qs);
						assert(fitQs.width <= maxCellSize.width);
						assert(fitQs.height <= maxCellSize.height);
						// 後でcand_flagにマークする為、配列に加えておく
						used.emplace_back(c.id);
						// 容量を超えた時点でループは終了
						remain -= fitQs.area();
						if(selected.size() >= param.avgImage+param::AuxImage() && remain <= 0)
							break;
						++i;
					}
					// まだ候補を最後まで調査してなければループを抜ける
					if(i < nCand) {
						break;
					}
					markUsed();
					// まだ隙間がある
				}
				if(!used.empty())
					markUsed();
				qDebug() << selected.size() << "Images selected";
				Q_ASSERT(selected.size() > 0);

				// 進捗は一旦0%にリセット
				emit sprinkleProgress(0);

				// 画像が一枚しかない場合は推奨サイズを適用
				if(selected.size() == 1) {
					Q_ASSERT(_state == State::Processing);
					_state = State::Idle;

					// 使用した画像にフラグを立てる
					_db->setViewFlag({selected.front().id}, 2);
					const place::ResultV res = {
						place::Result {
							.id = selected[0].id,
							.resize = ToQSize(lastRect.size()*qs),
							.crop = ToQSize(lastRect.size()*qs),
							.offset = {int(lastRect.offset().x*int(qs)), int(lastRect.offset().y*int(qs))},
						}
					};
					emit sprinkleResult(res);
					return;
				}
				// Geneスレッドへパラメータを送る(別スレッドで計算)
				QTimer::singleShot(0, _geneWorker, [
					worker = _geneWorker,
					sel = std::move(selected),
					ini = board,
					qs,
					n_img = param.avgImage
				](){
					worker->sprinkle(ini, sel, qs, n_img);
				});
			}
		);
	}
	void Sprinkler::sprinkle(const place::Param& param, const TagIdV& tag) {
		Q_ASSERT(_state == State::Idle);
		_state = State::WaitDelay;
		// (QLabelの削除が実際に画面へ適用されるまでタイムラグがある為)
		QTimer::singleShot(_const.delay_ms*2, this, [param, tag, this](){
			_sprinkle(param, tag);
		});
	}
	void Sprinkler::abort() {
		if(_state == State::Processing) {
			QMetaObject::invokeMethod(_geneWorker, "abort");
			_state = State::Aborted;
		} else if(_state == State::WaitDelay) {
			// QTimer::singleShotの後始末
			_state = State::Aborted;
		}
	}
	namespace {
		//! lst0からlst1の要素を除く
		template <class T0, class T1>
		void Exclude(T0& lst0, const T1& l1) {
			if(lst0.empty() || l1.empty())
				return;

			auto itr0 = lst0.begin(),
				 itr0E = lst0.end();
			while(itr0 != itr0E) {
				if(std::find(l1.begin(), l1.end(), *itr0) != l1.end()) {
					std::advance(itr0E, -1);
					if(itr0 == itr0E)
						break;
					*itr0 = std::move(*itr0E);
				} else
					++itr0;
			}
			lst0.erase(itr0, lst0.end());
		}
	}
	void Sprinkler::showImageContextMenu(const ImageId id, const QPoint& p) {
		QMenu menu;
		{
			// 現在リンクしているタグ
			const auto linkedTag = _db->getTagFromImage(id, false);
			if(!linkedTag.empty()) {
				for(auto tagId : linkedTag) {
					auto* a = menu.addAction(_db->getTagName(tagId));
					a->setEnabled(false);
				}
				menu.addSeparator();
			}
			{
				QMenu* sub = menu.addMenu(tr("&Link tag"));
				auto ru_tag = _db->getRecentryUsed(8, true);
				// 既に登録してあるタグは除く
				Exclude(ru_tag, linkedTag);
				if(!ru_tag.empty()) {
					sub->addSection(tr("Recentry used tag"));
					for(auto tagId : ru_tag) {
						auto* a = sub->addAction(_db->getTagName(tagId));
						connect(a, &QAction::triggered,
								_db, [db=_db, id, tagId](){
									db->makeTagLink(id, tagId);
								});
					}
					sub->addSeparator();
				}
				auto* a = sub->addAction(tr("input..."));
				connect(a, &QAction::triggered,
					_db, [db=_db, id](){
						auto* ti = new TagInput(db, db);
						connect(ti, &TagInput::accepted,
								db, [db, id](const TagIdV& tag){
									for(const auto tagId : tag)
										db->makeTagLink(id, tagId);
								});
						ti->show();
					});
			}
		}
		{
			// unlink可能なタグ
			const auto uTag = _db->getTagFromImage(id, true);
			if(!uTag.empty()) {
				QMenu* sub = menu.addMenu(tr("&Unlink tag"));
				for(auto tagId : uTag) {
					auto* a = sub->addAction(_db->getTagName(tagId));
					connect(a, &QAction::triggered,
						_db, [db=_db, id, tagId](){
							db->makeTagUnlink(id, tagId);
						});
				}
			}
		}
		menu.exec(p);
	}
}
