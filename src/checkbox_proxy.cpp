#include "checkbox_proxy.hpp"
#include "column_target.hpp"

namespace dg {
	CheckBoxProxy::CheckBoxProxy(ColumnTarget *const column, QObject *const parent):
		base_t(parent),
		_column(column)
	{}
	void CheckBoxProxy::sort(const int column, const Qt::SortOrder order) {
		if(_column->has(column)) {
			setSortRole(Qt::CheckStateRole);
		} else
			setSortRole(Qt::DisplayRole);
		base_t::sort(column, order);
	}
}
