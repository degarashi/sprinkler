#include "gene_worker.hpp"
#include "lubee/low_discrepancy.hpp"
#include "histgram/src/cell.hpp"
#include "histgram/src/maxrect.hpp"
#include "boolarray.hpp"

#include "genetic/src/gene_order.hpp"
#include "genetic/src/environment.hpp"
#include "genetic/src/pmx.hpp"
#include "genetic/src/jgg.hpp"
#include "genetic/src/bernoulli.hpp"
#include "genetic/src/swap.hpp"

#include <QStandardItem>
#include <random>
#include <QDebug>

namespace dg {
	namespace {
		struct SizePrio {
			lubee::SizeI	size;
			bool			important;
		};
		using SizePrioV = std::vector<SizePrio>;
		using SizeV = std::vector<lubee::SizeI>;

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
						MaxAsp = 16.f,
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
	void GeneWorker::calcArea(
		const dg::RequestParam& param,
		const dg::CellBoard& initial,
		const dg::PathS& keepset,
		const dg::PathS& notshown,
		const dg::ImageSet& img2size,
		const size_t qs
	){
		// アスペクト比とサイズの目安
		const auto asp = CalcAspSize(initial);
		const auto nAsp = asp.size();
		assert(nAsp > 0);

		const auto SampleR = param.nSample;
		const auto maxR = param.sizeRange.to;
		const auto minR = param.sizeRange.from;
		assert(minR <= maxR);

		// 空き面積から、適当に画像を選別
		SizePrioV				toPlace;
		int32_t					remain = initial.getNEmptyCell() * SampleR;
		PathV					selected;
		SizeV					modifiedSize;
		lubee::RectI			last;
		// Window環境でrandom_deviceを使うと毎回同じ乱数列が生成されてしまうので
		// とりあえずの回避策として現在時刻をシードに使う
		using Clock = std::chrono::system_clock;
		std::mt19937 mt(Clock::now().time_since_epoch().count());
		uint32_t				ld_cur = mt();

		const auto csz = initial.nboard().getSize();
		const auto addCandidate = [
			csz,
			&ld_cur,
			minR, maxR,
			&toPlace,
			qs,
			nAsp, &asp,
			&last,
			&modifiedSize,
			&selected,
			&remain,
			&img2size
		](const QString& path, const bool important){
			// 画像の元サイズ
			const QSize orig = *img2size.constFind(path);
			const float a = float(orig.width()) / orig.height();

			// アスペクト比を維持したまま拡大縮小して少くとも画面に一枚、配置できる目安サイズを検索
			last = asp.back().rect;
			lubee::SizeI maxsize = last.size() * qs;
			for(size_t i=0 ; i<nAsp ; i++) {
				auto& ent = asp[i];
				if(a <= ent.aspect) {
					last = ent.rect;
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
			}
			// } else {
				// // 拡大する
				// if(std::abs(orig.width() - maxsize.width) < std::abs(orig.height() - maxsize.height)) {
					// // 横幅をmaxsizeに合わせる
					// aspR = maxsize.width / float(orig.width());
				// } else {
					// // 縦幅をmaxsizeに合わせる
					// aspR = maxsize.height / float(orig.height());
				// }
			// }
			double lowd;
			lubee::Halton(&lowd, ld_cur++, 1);
			lowd = (maxR - minR) * lowd + minR;
			if(orig.width() < 384 && orig.height() < 384)
				lowd = 1;

			const float r = aspR * lowd;
			const int tagw2 = std::ceil(r * orig.width());
			const int tagh2 = std::ceil(r * orig.height());

			const lubee::SizeI sz{
				int((tagw2+qs-1)/qs),
				int((tagh2+qs-1)/qs)
			};
			assert(sz.width <= csz.width);
			assert(sz.height <= csz.height);
			remain -= int32_t(sz.area());

			selected.push_back(path);
			toPlace.emplace_back(SizePrio{sz, important});
			modifiedSize.emplace_back(tagw2, tagh2);
		};
		// 先にKeepした画像を入力
		for(auto&& path : keepset) {
			addCandidate(path, true);
		}
		auto itr = notshown.begin();
		while(itr != notshown.end() && remain > 0) {
			addCandidate(*itr, false);
			++itr;
		}
		qDebug() << selected.size() << "Selected";

		// 画像が一枚しかない場合は推奨サイズを適用
		if(selected.size() == 1) {
			PlaceV place = {
				PlaceResult{
					.resize = ToQSize(last.size()*qs),
					.crop = ToQSize(last.size()*qs),
					.offset = {int(last.offset().x*qs), int(last.offset().y*qs)},
					.path = selected[0]
				}
			};
			emit onProgress(100);
			emit geneResult(place);
			return;
		}

		// --------- GAで配置を最適化 ---------
		using Gene = gene::order::path::VariableGene<int>;
		using PMX = gene::order::cross::PartiallyMapped;
		using Mutate = gene::order::Bernoulli<gene::order::mutate::Swap>;
		using Env_t = gene::Environment<std::mt19937, Gene, MyFit, PMX, Mutate, gene::JustGenerationGap>;

		const size_t GeneLen = toPlace.size(),
					Population = 256,
					NParent = 16,
					NChild = 32;
		constexpr double MutateP = 0.01;
		MyFit fit(toPlace, initial, 1.0, 1.0);
		auto tmp = fit._initial;
		emit onProgress(0);
		Env_t env(mt, fit, {}, {MutateP, {}}, {NParent, NChild}, Population, GeneLen);
		const int NIter = 32;
		for(int i=0 ; i<NIter; i++) {
			env.advance();
			emit onProgress(int(std::floor(float(i+1)*100 / NIter)));
			std::cout << env.getBest().score << std::endl;
		}
		assert(tmp.nboard()  == fit._initial.nboard());

		const auto gene = env.getBest().gene;
		assert(gene.length() == GeneLen);

		PlaceV place;
		auto cb = initial;
		for(size_t i=0 ; i<GeneLen ; i++) {
			const auto idx = gene[i];
			const auto sz = toPlace[idx].size;
			if(cb.place(sz)) {
				// 画像をブロック枠サイズにピッタリ合わせる
				const lubee::SizeI szRect = sz * qs,
								szOriginal = modifiedSize[idx];
				const int dx = szRect.width - szOriginal.width,
						dy = szRect.height - szOriginal.height;
				Q_ASSERT(dx>=0 && dy>=0);
				// 高さ、幅の何れか足りない分が大きい方に合わせる
				float r;
				if(dx > dy) {
					r = float(szRect.width) / szOriginal.width;
				} else {
					r = float(szRect.height) / szOriginal.height;
				}
				const lubee::SizeI target {
					std::max<int>(szRect.width, szOriginal.width*r),
					std::max<int>(szRect.height, szOriginal.height*r)
				};
				Q_ASSERT(target.width >= szRect.width);
				Q_ASSERT(target.height >= szRect.height);
				const auto rect = cb.placedRect().back();
				auto ofs = rect.offset();
				ofs.x *= qs;
				ofs.y *= qs;

				place.push_back(
					PlaceResult {
						.resize = ToQSize(target),
						.crop = ToQSize(szRect),
						.offset = {ofs.x, ofs.y},
						.path = selected[idx]
					}
				);
			}
		}
		emit geneResult(place);
	}
}
