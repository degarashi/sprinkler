#pragma once
#include "lubee/range.hpp"
#include "lubee/point.hpp"
#include <QString>
#include <QSize>
#include <QHash>
#include <QSet>
#include <QVector>

namespace dg {
	struct RequestParam {
		lubee::RangeF	sizeRange;
		size_t			nSample;
	};
	using PathV = QVector<QString>;
	using PathS = QSet<QString>;
	using ImageSet = QHash<QString, QSize>;
}
