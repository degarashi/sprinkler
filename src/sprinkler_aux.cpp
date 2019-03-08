#include "sprinkler_aux.hpp"
#include "boolarray.hpp"
#include "histgram/src/maxrect.hpp"
#include "histgram/src/cell.hpp"

namespace dg {
	AspSizeV CalcAspSize(
		const CellBoard& qmap,
		const float minAsp,
		const float maxAsp,
		const float diffAsp
	) {
		const auto& nb = qmap.nboard();
		const auto qms = nb.getSize();
		struct OfsW {
			struct Hash {
				std::size_t operator()(const OfsW& u) const noexcept {
					return lubee::hash_combine_implicit(u.offset, u.width);
				}
			};
			lubee::PointI	offset;
			uint32_t		width;

			bool operator == (const OfsW& u) const noexcept {
				return offset == u.offset &&
						width == u.width;
			}
		};
		BoolArray ba(qms.width, qms.height);
		{
			auto* ba_p = ba.value.get();
			for(int i=0 ; i<qms.height ; i++) {
				for(int j=0 ; j<qms.width ; j++) {
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
		for(float asp=minAsp ; asp<maxAsp ; asp += diffAsp)
			as.emplace_back(AspSize{asp, calc(asp)});
		as.emplace_back(AspSize{maxAsp, calc(maxAsp)});
		return as;
	}
}
