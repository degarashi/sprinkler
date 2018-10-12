#pragma once
#include "lubee/point.hpp"
#include <QString>
#include <QSize>
#include <QHash>
#include <QSet>
#include <QVector>

namespace dg {
	using PathV = QVector<QString>;
	using PathS = QSet<QString>;
	using ImageSet = QHash<QString, QSize>;
}
