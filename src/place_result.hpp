#pragma once
#include "lubee/src/point.hpp"
#include <QString>
#include <QSize>

namespace dg {
	struct PlaceResult {
		QSize			resize;
		QSize			crop;
		lubee::PointI	offset;
		QString			path;
	};
	using PlaceV = QVector<PlaceResult>;
}
