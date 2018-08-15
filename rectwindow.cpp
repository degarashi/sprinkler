#include "rectwindow.hpp"
#include "ui_rectwindow.h"

namespace dg {
	rectwindow::rectwindow(QWidget *parent):
		QWidget(parent),
		_ui(new Ui::rectwindow)
	{
		_ui->setupUi(this);
		connect(_ui->quantizedView, SIGNAL(toggled(bool)), _ui->rectview, SLOT(setMode(bool)));
	}

	void rectwindow::onGridChanged(const dg::DomainSet& ds, const dg::CellBoard& qm, const size_t qs) {
		_ui->rectview->onGridChanged(ds, qm, qs);
	}
}
