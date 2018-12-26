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

#include <QApplication>
#include <QAction>
#include <random>
#include <QDebug>
#include <QTimer>
#include <QThread>

namespace dg {
	Sprinkler::Sprinkler():
		_toast(new ToastMgr(this)),
		_watcher(nullptr),
		_quantizer(nullptr),
		_qtntf(nullptr),
		_window{}
	{
		qApp->setQuitOnLastWindowClosed(false);
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
					ImageIdV used;
					for(auto& img : r) {
						used.emplace_back(img.id);
					}
					_db->setViewFlag(used, 2);
					// 候補には挙がったが使用されなかった画像のフラグをリセット
					_db->resetSelectionFlag();
					emit sprinkleResult(r);
				});
		connect(_geneWorker, &GeneWorker::sprinkleProgress,
				this, &Sprinkler::sprinkleProgress);
		connect(_db, &Database::endResetImage,
				this, &Sprinkler::imageChanged);
		getAction(Action::OpenMain)->toggle();
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
		constexpr size_t QuantifySize = 32,
						DelayMS = 200;
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

		_quantizer = new Quantizer(QuantifySize, this);
		_qtntf = new QtWNotifier(DelayMS, this);

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
			auto* w = new ImageDirWindow(db->makeDirModel(), _window.mainwin);
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
		const auto qs = QuantifySize;
		auto initial = _quantizer->qmap();
		// アスペクト比とサイズの目安
		const auto asp = CalcAspSize(initial, .25f, 16.f, .1f);
		const auto nAsp = asp.size();
		assert(nAsp > 0);

		// 残り面積
		auto remain = static_cast<int_fast32_t>(initial.getNEmptyCell() * param.nSample);
		// [低食い違い量列]
		// Window環境でrandom_deviceを使うと毎回同じ乱数列が生成されてしまうので
		// とりあえずの回避策として現在時刻をシードに使う
		using Clock = std::chrono::system_clock;
		std::mt19937 mt(static_cast<unsigned long>(Clock::now().time_since_epoch().count()));
		auto ld_cur = mt();
		// 最後に選択した矩形(quantized)
		lubee::RectI		lastRect;
		place::SelectedV	selected;
		ImageIdV			used;

		// 場のサイズ(チェック用)
		const auto csz = initial.nboard().getSize();
		const auto minR = param.sizeRange.from;
		const auto maxR = param.sizeRange.to;
		for(;;) {
			// アスペクト比均等で画像候補を列挙
			auto cand = _db->enumImageByAspect(tag, 5, 10);
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

				// アスペクト比を維持したまま拡大縮小して少くとも画面に一枚、配置できる目安サイズを検索
				lastRect = asp.back().rect;
				lubee::SizeI maxsize = lastRect.size() * qs;
				for(size_t i=0 ; i<nAsp ; i++) {
					auto& ent = asp[i];
					if(a <= ent.aspect) {
						lastRect = ent.rect;
						maxsize = ent.rect.size() * qs;
						break;
					}
				}

				float aspR = 1;
				if(orig.width() > maxsize.width) {
					// 横幅をmaxsizeに合わせる
					aspR = maxsize.width / float(orig.width());
				}
				if(orig.height() > maxsize.height) {
					// 縦幅をmaxsizeに合わせる
					aspR = std::min(aspR, maxsize.height / float(orig.height()));
				} else {
					// 拡大する
					// if(std::abs(orig.width() - maxsize.width) < std::abs(orig.height() - maxsize.height)) {
						// // 横幅をmaxsizeに合わせる
						// aspR = maxsize.width / float(orig.width());
					// } else {
						// // 縦幅をmaxsizeに合わせる
						// aspR = maxsize.height / float(orig.height());
					// }
				}

				// 低食い違い量列で適当にサイズ倍率を掛ける
				double lowd;
				lubee::Halton(&lowd, uint32_t(ld_cur++), 1);
				// 一定サイズより小さい画像はそのまま
				if(orig.width() < 384 && orig.height() < 384)
					lowd = 1;
				else
					lowd = double((maxR - minR) * float(lowd) + minR);		// Lerp(minR, maxR, lowd)

				const float r = aspR * float(lowd);
				const auto tagw2 = static_cast<int>(std::ceil(r * orig.width())),
							tagh2 = static_cast<int>(std::ceil(r * orig.height()));

				const lubee::SizeI sz{
					int((tagw2+int(qs)-1)/int(qs)),
					int((tagh2+int(qs)-1)/int(qs))
				};
				assert(sz.width <= csz.width);
				assert(sz.height <= csz.height);
				// 遺伝子候補に加える
				selected.emplace_back(
					place::Selected {
						.id = c.id,
						.modifiedSize = {tagw2, tagh2},
						.quantizedSize = sz,
						.important = false
					}
				);
				// 後でcand_flagにマークする為、配列に加えておく
				used.emplace_back(c.id);
				// 容量を超えた時点で終了
				remain -= int32_t(sz.area());
				if(remain <= 0)
					break;
				++i;
			}
			if(i < nCand || remain <= 0) {
				break;
			}
			// まだ隙間がある

			// 画像重複チェック
			{
				const size_t s0 = used.size();
				std::sort(used.begin(), used.end());
				used.erase(std::unique(used.begin(), used.end()), used.end());
				Q_ASSERT(s0 == used.size());
			}
			// 候補フラグを立てる
			_db->setViewFlag(used, 1);
			used.clear();
		}
		if(!used.empty()) {
			// 画像重複チェック
			{
				const size_t s0 = used.size();
				std::sort(used.begin(), used.end());
				used.erase(std::unique(used.begin(), used.end()), used.end());
				Q_ASSERT(s0 == used.size());
			}
			// 候補フラグを立てる
			_db->setViewFlag(used, 1);
			used.clear();
		}
		qDebug() << selected.size() << "Selected";
		Q_ASSERT(selected.size() > 0);

		// 画像が一枚しかない場合は推奨サイズを適用
		if(selected.size() == 1) {
			// 使用した画像にフラグを立てる
			_db->setViewFlag({selected.front().id}, 2);
			const place::ResultV res = {
				place::Result {
					.id = selected[0].id,
					.resize = ToQSize(lastRect.size()*qs),
					.crop = ToQSize(lastRect.size()*qs),
					.offset = {int(lastRect.offset().x*qs), int(lastRect.offset().y*qs)},
				}
			};
			emit sprinkleProgress(100);
			emit sprinkleResult(res);
			return;
		}
		// Geneスレッドへパラメータを送る
		QTimer::singleShot(0, _geneWorker, [
			worker = _geneWorker,
			sel = std::move(selected),
			ini = std::move(initial)
		](){
			worker->sprinkle(ini, sel, qs);
		});
	}
	void Sprinkler::sprinkle(const place::Param& param, const TagIdV& tag) {
		// (QLabelの削除が実際に画面へ適用されるまでタイムラグがある為)
		QTimer::singleShot(DelayMS*2, this, [param, tag, this](){
			_sprinkle(param, tag);
		});
	}
}