#pragma once
#include <QRect>
#include <QString>
#include <vector>

namespace dg {
	struct Rect_Name {
		QRect		rect;
		QString		name;
	};
	using Rect_NameV = std::vector<Rect_Name>;

	struct DomainType {
		enum {
			Screen,
			Qt,
			Other,
			_Num
		};
	};
	struct DomainSet {
		QRect			vscr;
		Rect_NameV		rv[DomainType::_Num];
	};
}
