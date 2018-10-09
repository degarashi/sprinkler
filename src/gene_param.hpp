#pragma once
#include "lubee/range.hpp"
#include "lubee/point.hpp"
#include <QString>
#include <QSize>
#include <QHash>
#include <unordered_set>

namespace dg {
	struct RequestParam {
		lubee::RangeF	sizeRange;
		size_t			nSample;
	};
	struct PlaceResult {
		QSize			resize;
		QSize			crop;
		lubee::PointI	offset;
		QString			path;
	};
	using PlaceV = std::vector<PlaceResult>;

	// DirModelのUserRole用
	struct ImageTag {
		QSize			size;
		QString			path;
		bool operator == (const ImageTag& t) const noexcept {
			return path == t.path;
		}
	};
	struct ImageTag_Hash {
		std::size_t operator()(const ImageTag& t) const noexcept {
			return qHash(t.path);
		}
	};
	using ImageV = std::vector<ImageTag>;
	using ImageSet = std::unordered_set<ImageTag, ImageTag_Hash>;
}
