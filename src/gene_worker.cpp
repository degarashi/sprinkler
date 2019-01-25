#include "gene_worker.hpp"
#include "lubee/src/rect.hpp"
#include "histgram/src/cell.hpp"
#include "src/place/selected.hpp"
#include "src/place/result.hpp"

#include "genetic/src/path/gene.hpp"
#include "genetic/src/environment.hpp"
#include "genetic/src/path/cross/pmx.hpp"
#include "genetic/src/generation/jgg.hpp"
#include "genetic/src/bernoulli.hpp"
#include "genetic/src/mutate/swap.hpp"
#include "genetic/src/mutate/uniform.hpp"
#include "genetic/src/real/cross/simplex.hpp"

#include <random>
#include <QRect>
#include <QCoreApplication>

namespace dg {
	namespace {
		const double MinSc = 0.25,
					MaxSc = 1.0;
		QSize ToQSize(const lubee::SizeI& s) noexcept {
			return {s.width, s.height};
		}
		struct Gene {
			using Path = gene::path::VariableGene<int>;
			using Scale = gene::VariableGene<double, gene::Vec<double>>;
			Path	path;
			Scale	scale;

			Gene() = default;
			Gene(const size_t n):
				path(n),
				scale(n)
			{}
			size_t length() const noexcept {
				assert(path.size() == scale.size());
				return path.size();
			}
			auto getIndex(const size_t i) const {
				return path[i];
			}
			auto getScale(const size_t i) const {
				return scale[i];
			}
			template <class RAND>
			static Gene MakeRandom(RAND& rd, const size_t len) {
				Gene ret(len);
				ret.path = Path::MakeRandom(rd, len);
				ret.scale = Scale::MakeRandom(rd, len, MinSc, MaxSc);
				return ret;
			}
		};
		struct Cross {
			using PMX = gene::path::cross::PartiallyMapped;
			using SPX = gene::real::cross::Simplex;

			std::bernoulli_distribution		_dist;
			bool							_sw;
			PMX								_pmx;
			SPX								_spx;
			size_t							_nParent;

			Cross(const size_t dim):
				_sw(false),
				_spx(dim)
			{}
			Cross(const size_t dim, const double eps):
				_sw(false),
				_spx(dim, eps)
			{}
			size_t prepare() noexcept {
				// path / scaleの切り替え
				_sw ^= 1;
				if(_sw)
					return _nParent = _pmx.prepare();
				return _nParent = _spx.prepare();
			}
			template <class RAND>
			std::vector<Gene> crossover(RAND& rd, const Gene** src) const {
				const auto nP = _nParent;
				assert(nP > 0);
				const auto len = src[0]->length();
				std::vector<Gene> g;
				if(_sw) {
					// 配置順序の交叉
					std::vector<const Gene::Path*> ptr{
						&src[0]->path,
						&src[1]->path
					};

					const auto path = _pmx.crossover(rd, ptr.data());
					assert(path.size() == nP);

					for(size_t i=0 ; i<nP ; i++) {
						g.emplace_back(Gene(len));
						auto& dst = g.back();
						dst.path = path[i];
						// スケーリング値はそれぞれで維持
						dst.scale.resize(len);
						for(size_t j=0 ; j<len ; j++) {
							dst.scale[j] = src[i]->scale[dst.path[j]];
						}
					}
				} else {
					// スケーリング値の交叉
					std::vector<const Gene::Scale*> ptr(nP);
					for(size_t i=0 ; i<nP ; i++)
						ptr[i] = &src[i]->scale;

					const auto scale = _spx.crossover(rd, ptr.data());
					assert(scale.size() == nP);

					for(size_t i=0 ; i<nP ; i++) {
						g.emplace_back(Gene(len));
						auto& dst = g.back();
						// 配置順序はコピー
						dst.path = src[i]->path;
						dst.scale = scale[i];
					}
				}
				return g;
			}
		};
		struct Mutate {
			std::bernoulli_distribution		_dist;
			gene::mutate::Uniform<double>	_unif;

			Mutate(const double prob_path, const double unifMin, const double unifMax):
				_dist(prob_path),
				_unif(unifMin, unifMax)
			{}
			template <class RAND, class Gene>
			void operator()(RAND& rd, Gene& g) {
				if(_dist(rd)) {
					gene::mutate::Swap()(rd, g.path);
				} else {
					_unif(rd, g.scale);
				}
			}
		};
		class Fit {
			public:
				place::SelectedV	_selected;
				CellBoard			_initial;
				double				_cfEmpty;
				size_t				_qs,
									_targetN;

			public:
				Fit(const place::SelectedV& selected, const CellBoard& initial,
						const double cfEmpty,
						const size_t qs, const size_t targetN
				):
					_selected(selected),
					_initial(initial),
					_cfEmpty(cfEmpty),
					_qs(qs),
					_targetN(targetN)
				{}
				template <class Gene>
				double operator()(const Gene& g) const {
					const auto len = g.length();
					assert(len == _selected.size());
					// 先頭から順に配置してみる
					auto cb = _initial;
					int nPlace = 0;
					for(size_t i=0 ; i<len ; i++) {
						auto& p = _selected[g.getIndex(i)];
						const auto qrect = p.getQuantizeScaledSize(g.getScale(i), _qs);
						if(cb.place(qrect))
							++nPlace;
					}
					double base = _targetN - std::min<int>(_targetN, std::abs(int(_targetN) - nPlace));
					base /= _targetN;
					return base + cb.getFillRatio() * _cfEmpty;
				}
		};
		struct Clip {
			void operator()(Gene& g) const noexcept {
				for(auto&& sc : g.scale) {
					sc = lubee::Saturate<double>(sc, MinSc, MaxSc);
					assert(lubee::IsInRange(sc, MinSc, MaxSc));
				}
			}
		};
	}
	void GeneWorker::abort() {
		_abort = true;
	}
	void GeneWorker::sprinkle(
		const dg::CellBoard& initial,
		const dg::place::SelectedV&	selected,
		const size_t qs,
		const size_t targetN
	){
		_abort = false;
		// --------- GAで配置を最適化 ---------
		using Mutate = gene::Bernoulli<Mutate>;
		using Pool_t = gene::Pool<Gene, Fit, Clip>;
		using Env_t = gene::Environment<
						std::mt19937,
						Gene,
						Pool_t,
						Cross,
						Mutate,
						gene::JustGenerationGap
						>;

		const size_t GeneLen = selected.size(),
					NParent = std::max<size_t>(16, GeneLen+1),
					NChild = std::max<size_t>(32, NParent*2),
					Population = NChild*2;
		constexpr double MutateP = 0.06,
						MutateP_Path = 0.5;
		Fit fit(selected, initial, 3.0, qs, targetN);
		auto tmp = fit._initial;
		emit sprinkleProgress(0);

		// Window環境でrandom_deviceを使うと毎回同じ乱数列が生成されてしまうので
		// とりあえずの回避策として現在時刻をシードに使う
		using Clock = std::chrono::system_clock;
		std::mt19937 mt(Clock::now().time_since_epoch().count());

		Env_t env(
			mt,
			Pool_t{mt, fit, Clip{}, Population, GeneLen},
			Cross{GeneLen},
			Mutate{MutateP, {MutateP_Path, MinSc, MaxSc}},
			gene::JustGenerationGap{NParent, NChild}
		);
		const int NIter = 32;
		double bestScore = std::numeric_limits<double>::lowest();
		Gene wg;
		for(int i=0 ; i<NIter; i++) {
			env.advance();
			emit sprinkleProgress(int(std::floor(float(i+1)*100 / NIter)));
			const auto s = env.getBest().score;
			std::cout << s << std::endl;
			if(bestScore < s) {
				wg = env.getBest().gene;
				bestScore = s;
			}
			QCoreApplication::processEvents();
			if(_abort) {
				emit sprinkleProgress(100);
				emit sprinkleAbort();
				return;
			}
		}
		assert(tmp.nboard()  == fit._initial.nboard());

		// これまでで一番優秀な遺伝子を使用
		const auto gene = wg;
		assert(gene.length() == GeneLen);

		place::ResultV res;
		auto cb = initial;
		for(size_t i=0 ; i<GeneLen ; i++) {
			const auto idx = gene.getIndex(i);
			const auto sz = selected[idx].getQuantizeScaledSize(gene.getScale(i), qs);
			if(cb.place(sz)) {
				// 画像をブロック枠サイズにピッタリ合わせる
				const lubee::SizeI szRect = sz * qs,
								szOriginal = selected[idx].fitSize * gene.getScale(i);
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
