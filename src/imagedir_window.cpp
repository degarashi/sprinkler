#include "imagedir_window.hpp"
#include "ui_imagedir_window.h"
#include "imagedir_model.hpp"
#include <QFileDialog>

namespace dg {
	ImageDirWindow::ImageDirWindow(ImageDirModel* model, QWidget *const parent) :
		GeomRestore_Widget("ImageDirWindow", parent),
		_ui(new Ui::ImageDirWindow),
		_model(model)
	{
		_ui->setupUi(this);
		_ui->dirview->setModel(model);
	}
	void ImageDirWindow::addDir() {
		const QString path(QFileDialog::getExistingDirectory(this, tr("Select Image Directory"), nullptr));
		// wineにて無効なディレクトリが返ってくる時があるのでそれを弾く
		// (またはDontUseNativeDialogを指定する)
		if(QFileInfo::exists(path)) {
			if(!path.isEmpty())
				_model->addDir(path);
		}
	}
	void ImageDirWindow::remDir() {
		// 単体選択時のみ対応
		const QModelIndexList sl = _ui->dirview->selectionModel()->selectedRows();
		if(sl.size() == 1)
			_model->removeDir(sl.first());
	}
}
