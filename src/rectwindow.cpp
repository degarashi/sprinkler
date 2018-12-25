#include "rectwindow.hpp"
#include "ui_rectwindow.h"

namespace dg {
	RectWindow::RectWindow(QWidget *parent):
		GeomRestore_Widget("RectWindow", parent),
		_ui(new Ui::RectWindow)
	{
		_ui->setupUi(this);
		connect(_ui->quantizedView, &QCheckBox::toggled,
				_ui->rectview, &RectView::setMode);
	}

	void RectWindow::onGridChanged(const dg::DomainSet& ds, const dg::CellBoard& qm, const size_t qs) {
		_ui->rectview->onGridChanged(ds, qm, qs);
	}
}
