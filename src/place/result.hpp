#pragma once
#include "lubee/src/point.hpp"
#include "../idtype.hpp"
#include <QSize>
#include <QVector>

namespace dg::place {
	struct Result {
		ImageId			id;
		QSize			resize;
		QSize			crop;
		lubee::PointI	offset;
	};
	using ResultV = QVector<Result>;
}
