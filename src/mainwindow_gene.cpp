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
#include "toast_mgr.hpp"

#include "gene_worker.hpp"
#include "ui_mainwindow.h"
#include "qtw_notifier.hpp"

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
		// 何も画像が残ってない場合はここで終了
		if(_notshown.empty() && _keepModel->rowCount() == 0) {
			mgr_toast.bake(
				Toast::Icon::Information,
				tr("No images"),
				tr("There's no image to show.\n(Image-set runs out)")
			);
			return;
		}
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
			_setControlsEnabled(false);

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
			QTimer::singleShot(0, _geneWorker,
				[
					worker = _geneWorker,
					param,
					cellb = _quantizer->qmap(),
					keepset = _keepSet,
					notshown = _notshown,
					img2size = _imageSet
				]()
				{
					worker->calcArea(param, cellb, keepset, notshown, img2size, QuantifySize);
				}
			);
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
