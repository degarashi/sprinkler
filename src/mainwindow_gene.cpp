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
		constexpr float MinAsp = .25f,
						MaxAsp = 4.f,
						DiffAsp = .1f;
		struct AspSize {
			float			aspect;
			lubee::RectI	rect;
		};
		using AspSizeV = std::vector<AspSize>;

		// アスペクト比とそれに対応する最大サイズ
		AspSizeV CalcAspSize(const CellBoard& qmap) {
			const auto& nb = qmap.nboard();
			const auto qms = nb.getSize();
			struct OfsW {
				struct Hash {
					std::size_t operator()(const OfsW& u) const noexcept {
						return lubee::hash_combine_implicit(u.offset, u.width);
					}
				};
				lubee::PointI	offset;
				int				width;

				bool operator == (const OfsW& u) const noexcept {
					return offset == u.offset &&
							width == u.width;
				}
			};
			BoolArray ba(qms.width, qms.height);
			{
				auto* ba_p = ba.value.get();
				for(size_t i=0 ; i<qms.height ; i++) {
					for(size_t j=0 ; j<qms.width ; j++) {
						*ba_p++ = nb.cellAt(j, i).used;
					}
				}
			}
			// [(Offset+Width) -> Height]
			std::unordered_map<OfsW, int, OfsW::Hash>		ow;
			// 最初に矩形の収集
			GetMaxRect(
				ba.value.get(),
				qms,
				[&ow](const lubee::RectI& r){
					ow[OfsW{r.offset(), r.width()}] = r.height();
				}
			);
			// アスペクト比に応じた最大サイズ計算
			const auto calc = [&ow](const float targetAsp) {
				float maxA = std::numeric_limits<float>::lowest();
				lubee::RectI rect;
				for(auto& o : ow) {
					const auto asp = float(o.first.width) / o.second;
					lubee::SizeI sz;
					if(targetAsp > asp) {
						sz.width = o.first.width;
						sz.height = std::floor(sz.width / targetAsp);
					} else {
						sz.height = o.second;
						sz.width = std::floor(sz.height * targetAsp);
					}
					const float a = sz.area();
					if(maxA < a) {
						maxA = a;
						rect = lubee::RectI{
							o.first.offset,
							sz
						};
					}
				}
				return rect;
			};
			AspSizeV as;
			for(float asp=MinAsp ; asp<MaxAsp ; asp += DiffAsp)
				as.emplace_back(AspSize{asp, calc(asp)});
			as.emplace_back(AspSize{MaxAsp, calc(MaxAsp)});
			return as;
		}
	}
	const size_t MainWindow::QuantifySize = 32;
	void MainWindow::_sprinkle() {
		_clearLabels();
		_cleanKeepList();
		// 何も画像が残ってない場合はここで終了
		if(_notshown.empty())
			return;
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
				wk->calcArea(param, cb, keep, ns, QuantifySize);
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
