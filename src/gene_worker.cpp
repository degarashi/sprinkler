#include "gene_worker.hpp"
#include "lubee/src/rect.hpp"
#include "histgram/src/cell.hpp"
#include "src/place/selected.hpp"
#include "src/place/result.hpp"

#include "genetic/src/gene_order.hpp"
#include "genetic/src/environment.hpp"
#include "genetic/src/pmx.hpp"
#include "genetic/src/jgg.hpp"
#include "genetic/src/bernoulli.hpp"
#include "genetic/src/swap.hpp"

#include <random>
#include <QRect>

namespace dg {
	namespace {
		QRect ToQRect(const lubee::RectI& r) {
			return {
				r.x0,
				r.y0,
				r.width(),
				r.height()
			};
		}
		QSize ToQSize(const lubee::SizeI& s) noexcept {
			return {s.width, s.height};
		}
		class Fit {
			public:
				place::SelectedV	_selected;
				CellBoard			_initial;
				double				_cfNet,
									_cfEmpty;

			public:
				Fit(const place::SelectedV& selected, const CellBoard& initial,
						const double cfNet, const double cfEmpty):
					_selected(selected),
					_initial(initial),
					_cfNet(cfNet),
					_cfEmpty(cfEmpty)
				{}
				template <class Gene>
				double operator()(const Gene& g) const {
					double score = 0;
					const auto len = g.length();
					assert(len == _selected.size());
					auto cb = _initial;
					for(size_t i=0 ; i<len ; i++) {
						auto& p = _selected[g[i]];
						// Keepした画像を入れてない場合は大幅減点
						if(!cb.place(p.quantizedSize)) {
							if(p.important)
								score -= 10000;
						}
					}
					return score + cb.calcScore(_cfNet, _cfEmpty);
				}
		};
	}
	void GeneWorker::sprinkle(
		const dg::CellBoard& initial,
		const dg::place::SelectedV&	selected,
		const size_t qs
	){
		// --------- GAで配置を最適化 ---------
		using Gene = gene::order::path::VariableGene<int>;
		using PMX = gene::order::cross::PartiallyMapped;
		using Mutate = gene::order::Bernoulli<gene::order::mutate::Swap>;
		using Env_t = gene::Environment<std::mt19937, Gene, Fit, PMX, Mutate, gene::JustGenerationGap>;

		const size_t GeneLen = selected.size(),
					Population = 256,
					NParent = 16,
					NChild = 32;
		constexpr double MutateP = 0.01;
		Fit fit(selected, initial, 1.0, 1.0);
		auto tmp = fit._initial;
		emit sprinkleProgress(0);

		// Window環境でrandom_deviceを使うと毎回同じ乱数列が生成されてしまうので
		// とりあえずの回避策として現在時刻をシードに使う
		using Clock = std::chrono::system_clock;
		std::mt19937 mt(Clock::now().time_since_epoch().count());

		Env_t env(mt, fit, {}, {MutateP, {}}, {NParent, NChild}, Population, GeneLen);
		const int NIter = 32;
		for(int i=0 ; i<NIter; i++) {
			env.advance();
			emit sprinkleProgress(int(std::floor(float(i+1)*100 / NIter)));
			std::cout << env.getBest().score << std::endl;
		}
		assert(tmp.nboard()  == fit._initial.nboard());

		const auto gene = env.getBest().gene;
		assert(gene.length() == GeneLen);

		place::ResultV res;
		auto cb = initial;
		for(size_t i=0 ; i<GeneLen ; i++) {
			const auto idx = gene[i];
			const auto sz = selected[idx].quantizedSize;
			if(cb.place(sz)) {
				// 画像をブロック枠サイズにピッタリ合わせる
				const lubee::SizeI szRect = sz * qs,
								szOriginal = selected[idx].modifiedSize;
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

				res.push_back(
					place::Result {
						.id = selected[idx].id,
						.resize = ToQSize(target),
						.crop = ToQSize(szRect),
						.offset = {ofs.x, ofs.y}
					}
				);
			}
		}
		emit sprinkleResult(res);
	}
}
