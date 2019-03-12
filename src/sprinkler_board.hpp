#pragma once
#include "lubee/src/rect.hpp"
#include "spine/src/enum.hpp"

namespace dg {
	class NBoard;
	namespace place {
		struct Result;
		using ResultV = std::vector<Result>;
	}
	class SprBoard {
		private:
			using AreaId = size_t;
			using cell_t = uint8_t;
			using array_t = std::vector<cell_t>;
			constexpr static cell_t Wall{cell_t(~0)},
									Empty{cell_t(0)};
			struct Log {
				lubee::RectI	rect;
				struct {
					array_t		data;
					size_t		nEmpty;
				} previous;
			};

			array_t			_ar;
			// 周囲一マスを含んだ状態のサイズ
			lubee::SizeI	_size;
			size_t			_nEmpty;

			// 空き領域に番号をつける(1から通し番号)
			// 最大の番号を返す。0の時は空き無し
			AreaId _assignEmptyNum();
			// 矩形の伸長を適用 -> 前の状態をLogに記録
			Log _applyExpand(const lubee::RectI& rFrom,
							const lubee::RectI& rTo);
			void _fillWall(lubee::RectI rect);
			void _applyLog(const Log& log);
			// デバッグ用途
			bool operator == (const SprBoard& b) const noexcept;
			cell_t& _cellAt(size_t x, size_t y) noexcept;
			const cell_t& _at(lubee::PointI p) const noexcept;
			cell_t& _at(lubee::PointI p) noexcept;

			DefineEnum(
				Dir4,
				(Left)
				(Top)
				(Right)
				(Bottom)
			);
			using Dir4_O = std::optional<Dir4>;
			DefineEnumPair(
				Dir4F,
				((Left)(1 << Dir4::Left))
				((Top)(1 << Dir4::Top))
				((Right)(1 << Dir4::Right))
				((Bottom)(1 << Dir4::Bottom))
			);
			struct SweepDiff {
				lubee::PointI	local,
								global;
			};
			// 方向から走査ごとのカーソル移動距離を取得
			const static SweepDiff c_sweepDiff[Dir4::_Num];
			struct SweepPtLen {
				lubee::PointI	pt;
				size_t			localLength;
			};
			using SFunc = std::function<SweepPtLen (const lubee::RectI&)>;
			// 矩形から走査開始の座標を取得
			const static SFunc c_sweepBegin[Dir4::_Num];
			static bool _ExpandRect(lubee::RectI& r, Dir4 dir, size_t n);

			using RectId = size_t;
			struct Cand {
				RectId		rectId;
				AreaId		neighborAreaId;
				Dir4		dir0;
				Dir4_O		dir1;

				bool operator == (const Cand& c) const noexcept;
				size_t hashValue() const noexcept;
			};
			using CandV = std::vector<Cand>;
			using CandV2 = std::vector<CandV>;
			// 小島毎に候補を振り分け
			CandV2 _allocIsland(const place::ResultV& result);
			// 候補の展開
			template <class CB>
			void _expandCandidate(const place::ResultV& res, const Cand& c, CB&& cb);

			bool _checkValidness() const noexcept;

		public:
			SprBoard(const NBoard& board);

			bool hasEmptyCell() const noexcept;
			void fillInEmptyCells(place::ResultV& result);
			size_t nEmpty() const noexcept;
	};
}
