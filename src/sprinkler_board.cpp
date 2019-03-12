#include "sprinkler_board.hpp"
#include "histgram/src/nboard.hpp"
#include "place/result.hpp"
#include "lubee/src/recursive_lambda.hpp"
#include "lubee/src/unionset.hpp"
#include <iostream>
#include <unordered_set>

namespace dg {
	namespace {
		constexpr size_t MaxCandidateSize = 8;
		struct HashValue {
			template <class T>
			size_t operator()(const T& t) const noexcept {
				return t.hashValue();
			}
		};
	}
	bool SprBoard::Cand::operator == (const Cand& c) const noexcept {
		return rectId == c.rectId
			&& neighborAreaId == c.neighborAreaId
			&& dir0 == c.dir0
			&& dir1 == c.dir1;
	}
	size_t SprBoard::Cand::hashValue() const noexcept {
		return std::hash<RectId>()(rectId)
			* std::hash<AreaId>()(neighborAreaId)
			+ std::hash<Dir4::value_t>()(dir0)
			^ std::hash<std::optional<Dir4::value_t>>()(dir1);
	}
	const SprBoard::SweepDiff SprBoard::c_sweepDiff[Dir4::_Num] = {
		{{0, 1}, {-1, 0}},
		{{1, 0}, {0, -1}},
		{{0, 1}, {1, 0}},
		{{1, 0}, {0, 1}},
	};
	const SprBoard::SFunc SprBoard::c_sweepBegin[Dir4::_Num] = {
		[](const lubee::RectI& r) -> SweepPtLen {
			return {{r.x0-1, r.y0}, r.height()}; },
		[](const lubee::RectI& r) -> SweepPtLen {
			return {{r.x0, r.y0-1}, r.width()}; },
		[](const lubee::RectI& r) -> SweepPtLen {
			return {{r.x1, r.y0}, r.height()}; },
		[](const lubee::RectI& r) -> SweepPtLen {
			return {{r.x0, r.y1}, r.width()}; },
	};
	bool SprBoard::_ExpandRect(lubee::RectI& r, const Dir4 dir, const size_t n) {
		using RFunc = std::function<void(lubee::RectI&, size_t)>;
		static const RFunc c_rectExpand[] = {
			[](lubee::RectI& r, const size_t n) { r.x0 -= int(n); },
			[](lubee::RectI& r, const size_t n) { r.y0 -= int(n); },
			[](lubee::RectI& r, const size_t n) { r.x1 += int(n); },
			[](lubee::RectI& r, const size_t n) { r.y1 += int(n); }
		};
		c_rectExpand[dir](r, n);
		return false;
	}

	SprBoard::cell_t& SprBoard::_cellAt(const size_t x, const size_t y) noexcept {
		return _ar[y * _size.width + x];
	}
	void SprBoard::_fillWall(const lubee::RectI rect) {
		auto nEmp = _nEmpty;
		for(int i=rect.y0 ; i<rect.y1 ; i++) {
			for(int j=rect.x0 ; j<rect.x1 ; j++) {
				auto& a = _at({j, i});
				D_Assert0(a != Wall);
				--nEmp;
				a = Wall;
			}
		}
		_nEmpty = nEmp;
		D_Assert0(_checkValidness());
	}
	void SprBoard::_applyLog(const Log& log) {
		D_Assert0(_nEmpty < log.previous.nEmpty);

		const auto r = log.rect;
		const auto ofs = r.offset();
		for(size_t i=0 ; i<r.height() ; i++) {
			for(size_t j=0 ; j<r.width() ; j++) {
				auto& dst = _at(lubee::PointI{int(j), int(i)} + ofs);
				dst = log.previous.data[i*r.width() + j];
			}
		}
		_nEmpty = log.previous.nEmpty;
		D_Assert0(_checkValidness());
	}
	bool SprBoard::_checkValidness() const noexcept {
		size_t nEmp = 0;
		for(auto&& a : _ar) {
			if(a != Wall)
				++nEmp;
		}
		return nEmp == _nEmpty;
	}
	SprBoard::Log SprBoard::_applyExpand(
			const lubee::RectI& rFrom,
			const lubee::RectI& rTo
	) {
		D_Assert0(
			lubee::IsInRange(
				bool(rFrom.x0 == rTo.x0)
				+ bool(rFrom.x1 == rTo.x1)
				+ bool(rFrom.y0 == rTo.y0)
				+ bool(rFrom.y1 == rTo.y1),
				2,3
			)
		);

		Log log;
		log.rect = {
			std::min(rFrom.x0, rTo.x0),
			std::max(rFrom.x1, rTo.x1),
			std::min(rFrom.y0, rTo.y0),
			std::max(rFrom.y1, rTo.y1)
		};
		log.previous.data.resize(log.rect.area());
		log.previous.nEmpty = _nEmpty;

		auto nEmp = _nEmpty;
		const auto& r = log.rect;
		const auto ofs = r.offset();
		for(size_t i=0 ; i<r.height() ; i++) {
			for(size_t j=0 ; j<r.width() ; j++) {
				auto& dst = _at(lubee::PointI{int(j), int(i)} + ofs);
				log.previous.data[i*r.width() + j] = dst;
				if(dst != Wall) {
					D_Assert0(nEmp > 0);
					--nEmp;
					dst = Wall;
				}
			}
		}
		D_Assert0(_nEmpty > nEmp);
		_nEmpty = nEmp;
		D_Assert0(_checkValidness());
		return log;
	}
	SprBoard::SprBoard(const NBoard& b) {
		_nEmpty = b.getNEmptyCell();
		const auto sr = b.getSize();

		// 周りを1マスの壁で覆うので+2
		auto sw = sr;
		sw.width += 2;
		sw.height += 2;
		_size = sw;

		_ar.resize(size_t(sw.area()));
		// 周囲の壁を作成
		{
			// 縦
			for(size_t i=0 ; i<sw.height ; i++) {
				// 左
				_cellAt(0, i) = Wall;
				// 右
				_cellAt(sw.width-1, i) = Wall;
			}
			// 横
			for(size_t i=0 ; i<sw.width ; i++) {
				// 上
				_cellAt(i, 0) = Wall;
				// 下
				_cellAt(i, sw.height-1) = Wall;
			}
		}
		// その他
		for(int i=0 ; i<int(sr.height) ; i++) {
			for(int j=0 ; j<int(sr.width) ; j++) {
				_at({j, i}) = b.cellAt(j, i).used ? Wall : Empty;
			}
		}
		D_Assert0(_checkValidness());
	}
	size_t SprBoard::_assignEmptyNum() {
		// 空き領域に番号をつける(1から通し番号)
		const auto s = _size;
		// 周囲のマスの探索(Filling)
		const lubee::Recursive r(
			[this](auto&& f, const cell_t num, const size_t x, const size_t y) -> void{
				auto& b = _cellAt(x, y);
				if(b != Empty)
					return;
				D_Assert0(x>0 && y>0);
				D_Assert0(b <= num);
				b = num;

				f(num, x+1, y);
				f(num, x-1, y);
				f(num, x, y-1);
				f(num, x, y+1);
			}
		);

		size_t num = 0;
		for(size_t i=0 ; i<s.height ; i++) {
			for(size_t j=0 ; j<s.width ; j++) {
				if(_cellAt(j, i) == Empty) {
					++num;
					D_Assert0(num < Wall);
					r(num, j, i);
				}
			}
		}
		return num;
	}
	bool SprBoard::operator == (const SprBoard& b) const noexcept {
		return _size == b._size
				&& _nEmpty == b._nEmpty
				&& _ar == b._ar;
	}
	bool SprBoard::hasEmptyCell() const noexcept {
		return nEmpty() == 0;
	}
	size_t SprBoard::nEmpty() const noexcept {
		return _nEmpty;
	}
	SprBoard::cell_t& SprBoard::_at(const lubee::PointI pt) noexcept {
		return _ar[size_t(pt.y+1) * _size.width + size_t(pt.x+1)];
	}
	const SprBoard::cell_t& SprBoard::_at(const lubee::PointI pt) const noexcept {
		return const_cast<SprBoard*>(this)->_at(pt);
	}
	SprBoard::CandV2 SprBoard::_allocIsland(const place::ResultV& result) {
		// Resultの矩形で領域を埋める
		for(auto& r : result) {
			_fillWall(r.rect);
			// 面積ゼロの矩形は不正
			D_Assert0(r.rect.area() > 0);
		}
		// 空き領域に番号を割り振る
		const auto maxEmptyNum = _assignEmptyNum();
		if(maxEmptyNum == 0) {
			// 空き領域は無いのでこれ以上処理しない
			return {};
		}

		CandV			cand;
		lubee::UnionSet us(static_cast<unsigned int>(maxEmptyNum+1));
		// 各矩形がどの方向に伸長可能か(+ 隣接AreaId)を調べる
		int cur = 0;
		for(auto& r : result) {
			const auto canExpand = [this](
				const lubee::RectI& rect,
				const Dir4 dir
			) -> std::optional<AreaId> {
				auto sw = c_sweepBegin[dir](rect);
				const auto diff = c_sweepDiff[dir];
				const AreaId areaId = _at(sw.pt);
				for(size_t i=0 ; i<sw.localLength ; i++) {
					if(_at(sw.pt) == Wall)
						return std::nullopt;
					sw.pt += diff.local;
				}
				return areaId;
			};
			auto& rect = r.rect;

			AreaId areaId[2];
			Dir4 dir[2];
			int n = 0;

			for(Dir4::value_t i=0 ; i<Dir4::_Num ; i++) {
				const Dir4 d = static_cast<Dir4::e>(i);
				if(const auto aId = canExpand(rect, d)) {
					areaId[n] = *aId;
					dir[n] = d;
					++n;
					// 矩形が角に寄せられていない(三辺以上が伸長可能)とエラー
					D_Assert0(n <= 2);
				}
			}
			#ifdef DEBUG
				// 矩形が角に寄せられていない(上下や左右に伸長可能)とエラー
				if(n == 2) {
					const Dir4F flag = (1<<dir[0]) | (1<<dir[1]);
					D_Assert0((flag & Dir4F::Left) ^ (flag & Dir4F::Right));
					D_Assert0((flag & Dir4F::Top) ^ (flag & Dir4F::Bottom));
				}
			#endif
			if(n > 0) {
				Cand c;
				c.rectId = RectId(cur);
				c.dir0 = dir[0];
				c.neighborAreaId = areaId[0];
				if(n == 2) {
					// 2つの領域を統合
					us.merge(
						static_cast<unsigned int>(areaId[0]),
						static_cast<unsigned int>(areaId[1])
					);
					c.dir1 = dir[1];
				}
				// 候補に加える
				cand.emplace_back(c);
			}
			++cur;
		}

		// 小島毎に候補を振り分け
		using Island = std::unordered_map<AreaId, CandV>;
		Island island;
		for(auto& c : cand) {
			island[us.getRoot(static_cast<unsigned int>(c.neighborAreaId))].emplace_back(c);
		}
		CandV2 ret;
		for(auto& isl : island) {
			ret.emplace_back(std::move(isl.second));
		}
		return ret;
	}
	namespace {
		struct EmptyClass {
			template <class... Args>
			EmptyClass(const Args&...) {}
		};
	}
	template <class CB>
	void SprBoard::_expandCandidate(const place::ResultV& res, const Cand& c, CB&& cb) {
		// 線分が任意の方向にいくら伸長可能か調べる
		const auto expandLen = [this](
			const lubee::RectI& r,
			const Dir4 dir
		) {
			const auto ptlen = c_sweepBegin[dir](r);
			const auto diff = c_sweepDiff[dir];
			size_t len = 0;
			auto pt = ptlen.pt;
			for(;;) {
				auto ptL = pt;
				for(size_t i=0 ; i<ptlen.localLength ; i++) {
					if(_at(ptL) == Wall)
						return len;
					ptL += diff.local;
				}
				++len;
				pt += diff.global;
			}
		};
		const auto cornerIsWall = [this](
			const lubee::RectI& rect,
			const Dir4F flag
		) {
			lubee::PointI pt;
			if(flag & Dir4F::Left)
				pt.x = rect.x0-1;
			else if(flag & Dir4F::Right)
				pt.x = rect.x1;
			else {
				D_Assert0(false);
			}

			if(flag & Dir4F::Top)
				pt.y = rect.y0-1;
			else if(flag & Dir4F::Bottom)
				pt.y = rect.y1;
			else {
				D_Assert0(false);
			}

			return _at(pt) == Wall;
		};
		const auto& rect = res[c.rectId].rect;
		struct DLen {
			Dir4		dir;
			size_t		len;
		};
		DLen valid[2];
		int nValid = 0;
		if((valid[nValid].len = expandLen(rect, c.dir0)) > 0)
			valid[nValid++].dir = c.dir0;
		if(c.dir1) {
			if((valid[nValid].len = expandLen(rect, *c.dir1)) > 0)
				valid[nValid++].dir = *c.dir1;
		}
		D_Assert0(nValid <= 2);

		const auto callCB = [&rect, &cb](const auto... dlen){
			auto trect = rect;
			EmptyClass{_ExpandRect(trect, dlen.dir, dlen.len)...};
			cb(trect);
		};
		if(nValid == 2 && !cornerIsWall(rect, (1<<valid[0].dir) | (1<<valid[1].dir))) {
			// 斜めの場合はdir0->dir1, dir1->dir0の2通り調べる
			callCB(valid[0], valid[1]);
			callCB(valid[1], valid[0]);
		} else if(nValid == 1) {
			// 候補は1つだけ
			callCB(valid[0]);
		}
	}
	void SprBoard::fillInEmptyCells(place::ResultV& result) {
		auto c2 = _allocIsland(result);

		struct Procedure {
			RectId			rectId;
			lubee::RectI	transformTo;
		};
		using ProcedureV = std::vector<Procedure>;
		struct RecursiveResult {
			size_t		remain;
			ProcedureV	procedure;
		};
		using CandSet = std::unordered_set<Cand, HashValue>;
		const lubee::Recursive rec([&result](
			auto&& rec,
			SprBoard& board,
			ProcedureV& procedure,
			CandSet& candSet
		) -> RecursiveResult {
				// 全ての候補を探索し終わったので結果を返す
				if(candSet.empty()) {
					return {
						board.nEmpty(),
						procedure
					};
				}
				RecursiveResult res;
				res.remain = std::numeric_limits<size_t>::max();

				auto tCandSet = candSet;
				// 候補集合から１つ取り出し、適用、次へ
				for(auto& c : candSet) {
					auto itr = tCandSet.find(c);
					tCandSet.erase(itr);

					// アクションを実行出来た場合の探索
					board._expandCandidate(result, c, [&](const lubee::RectI& rect){
						procedure.emplace_back(
							Procedure{
								c.rectId,
								rect
							}
						);
						const auto log = board._applyExpand(
							result[c.rectId].rect,
							rect
						);
						#ifdef DEBUG
							const auto tmp = board;
						#endif
						const auto resL = rec(
							board,
							procedure,
							tCandSet
						);
						D_Assert0(board == tmp);
						if(res.remain > resL.remain)
							res = resL;
						board._applyLog(log);
						procedure.pop_back();
					});
					// アクションを実行できなかった(しなかった)場合の探索
					{
						#ifdef DEBUG
							const auto tmp = board;
						#endif
						const auto resL = rec(board, procedure, tCandSet);
						D_Assert0(board == tmp);
						if(res.remain > resL.remain)
							res = resL;
					}
					tCandSet.emplace(c);
				}
				return res;
			}
		);
		// 小島毎に最適解を探索
		auto tboard = *this;
		for(auto& cand : c2) {
			// 候補の数が多いと組み合わせ爆発を起こすので一定数に制限
			if(cand.size() > MaxCandidateSize)
				continue;
			ProcedureV proc;
			{
				CandSet cs;
				for(auto& c : cand)
					cs.emplace(c);
				proc = rec(tboard, proc, cs).procedure;
			}
			D_Assert0(*this == tboard);

			// Resultの改変
			for(auto& p : proc) {
				std::cout << result[p.rectId].rect << " -> " << p.transformTo << std::endl;
				result[p.rectId].rect = p.transformTo;
			}
		}
	}
}
