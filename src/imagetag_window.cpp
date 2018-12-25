#include "imagetag_window.hpp"
#include "ui_imagetag_window.h"
#include "dbimage_if.hpp"
#include "tagselect_model.hpp"
#include "column_target.hpp"
#include "imagequery_model.hpp"
#include "checkbox_delegate.hpp"
#include "checkbox_proxy.hpp"
#include "spine/src/enum.hpp"
#include <QComboBox>
#include <QSqlRelationalTableModel>

namespace dg {
	ImageTagWindow::ImageTagWindow(DBTag* tag, DBImage* img, DatabaseSignal* sig, QWidget *parent):
		GeomRestore_Widget("ImageTagWindow", parent),
		_ui(new Ui::ImageTagWindow),
		_dirty(true)
	{
		auto [imgMdl, imgQMdl, dlg] = img->makeImageModel(this);
		_result = imgQMdl;

		_ui->setupUi(this);
		_ui->tagSelector->init(tag, sig);
		this->layout()->update();
		connect(_ui->tagSelector, &widget::TagSelector::changed,
				this, [this](){
					_setDirty();
				});
		_ui->resultlist->setModel(imgMdl);
		_ui->resultlist->setItemDelegate(dlg);
		_ui->resultlist->verticalHeader()->setHidden(true);
		_ui->resultlist->horizontalHeader()->setStretchLastSection(true);

		_resetDirty();
	}
	bool ImageTagWindow::_setDirty() {
		if(!_dirty) {
			_dirty = true;
			_ui->pbSearch->setEnabled(_dirty);
			return true;
		}
		return false;
	}
	bool ImageTagWindow::_resetDirty() {
		if(_dirty) {
			_dirty = false;
			_ui->pbSearch->setEnabled(_dirty);
			return true;
		}
		return false;
	}
	void ImageTagWindow::onSearch() {
		if(_resetDirty()) {
			// タグ番号配列をモデルにセットして後は任せる
			_result->setTagFilter(_ui->tagSelector->getArray());
		}
	}
}
