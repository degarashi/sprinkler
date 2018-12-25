#pragma once
#include <QRect>
#include <QString>
#include <vector>

namespace dg {
	struct Domain {
		QRect		rect;
		QString		name;
	};
	using DomainV = std::vector<Domain>;

	struct DomainSet {
		QRect			vscr;
		struct {
			DomainV		screen,
						qt,
						other;
		} domain;
	};
}
