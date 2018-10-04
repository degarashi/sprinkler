#include "mainwindow.hpp"
#include "quantizer.hpp"
#include "lubee/low_discrepancy.hpp"

#include "genetic/src/gene_order.hpp"
#include "genetic/src/environment.hpp"
#include "genetic/src/pmx.hpp"
#include "genetic/src/jgg.hpp"
#include "genetic/src/bernoulli.hpp"
#include "genetic/src/swap.hpp"
#include "histgram/src/fit_cell.hpp"
#include "histgram/src/maxrect.hpp"
#include "boolarray.hpp"

#include "glabel.hpp"
#include <QDebug>
#include <QTimer>
#include <QStandardItemModel>
#include <QApplication>
#include <QStyle>
#include <QFileInfo>
#include <QImageReader>

#include "gene_worker.hpp"
#include "ui_mainwindow.h"
#include "qtw_notifier.hpp"

Q_DECLARE_METATYPE(dg::KeepData)
namespace dg {
	namespace {
		struct SizePrio {
			lubee::SizeI	size;
			QModelIndex		index;
			bool			important;
		};
		using SizePrioV = std::vector<SizePrio>;

		QSize ToQSize(const lubee::SizeI& s) noexcept {
			return {s.width, s.height};
		}
		QRect ToQRect(const lubee::RectI& r) {
			return {
				r.x0,
				r.y0,
				r.width(),
				r.height()
			};
		}
		class MyFit {
			public:
				SizePrioV		_toPlace;
				CellBoard		_initial;
				double			_cfNet,
								_cfEmpty;

			public:
				MyFit(const SizePrioV& toPlace, const CellBoard& initial,
						const double cfNet, const double cfEmpty):
					_toPlace(toPlace),
					_initial(initial),
					_cfNet(cfNet),
					_cfEmpty(cfEmpty)
				{}
				template <class Gene>
				double operator()(const Gene& g) const {
					double score = 0;
					const auto len = g.length();
					assert(len == _toPlace.size());
					auto cb = _initial;
					for(size_t i=0 ; i<len ; i++) {
						auto& p = _toPlace[g[i]];
						// Keepした画像を入れてない場合は大幅減点
						if(!cb.place(p.size)) {
							if(p.important)
								score -= 10000;
						}
					}
					return score + cb.calcScore(_cfNet, _cfEmpty);
				}
		};
	}
	const size_t MainWindow::QuantifySize = 32;
	void MainWindow::_sprinkle() {
		_clearLabels();
		_cleanKeepList();
		// 何も画像が残ってない場合はここで終了
		if(_notshown.empty())
			return;
		// Hide window when sprinkleがONの時はここでウィンドウを隠す
		if(_ui->actionHideWhenSprinkle->isChecked()) {
			setWindowState(Qt::WindowMinimized);
			QMetaObject::invokeMethod(
				this,
				[this](){
					QMetaObject::invokeMethod(_qtntf, &QtWNotifier::onQtGeometryChanged);
					// DirListWindowとWatchListWindowも閉じる
					_ui->actionOpenDirList->setChecked(false);
					_ui->actionOpenWatchList->setChecked(false);
				}
			);
		}
		// (QLabelの削除が実際に画面へ適用されるまでタイムラグがある為)
		QTimer::singleShot(10, this, [this](){
			const auto getData = [model = _reqModel](const int idx, auto type) {
				return model->data(model->index(idx,0), Qt::EditRole).value<decltype(type)>();
			};
			const RequestParam param {
				.sizeRange = {
					getData(Request::Min, float()),
					getData(Request::Max, float()),
				},
				.nSample = getData(Request::Sample, size_t())
			};
			ImageSet keep;
			{
				const int nR = _keepModel->rowCount();
				for(int i=0 ; i<nR ; i++) {
					const QModelIndex idx = _keepModel->index(i,0);
					const auto kp = _keepModel->data(idx, Qt::UserRole).value<KeepData>();
					keep.emplace(ImageTag{
						.size = QImageReader(kp.path).size(),
						.path = kp.path
					});
					_path2idx.insert(kp.path, idx);
				}
			}
			_setControlsEnabled(false);
			QTimer::singleShot(0, _geneWorker, [wk=_geneWorker, param, keep, cb=_quantizer->qmap(), ns=_notshown](){
				wk->calcArea(param, cb, keep, std::move(ns), QuantifySize);
			});
		});
	}
	void MainWindow::reSprinkle() {
		_shown = _shownP;
		_notshown = _notshownP;
		_sprinkle();
	}
	void MainWindow::sprinkle() {
		_shownP = _shown;
		_notshownP = _notshown;
		_sprinkle();
	}
}
