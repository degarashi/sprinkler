#include "imagedir_window.hpp"
#include "ui_imagedir_window.h"
#include "imagedir_model.hpp"
#include <QFileDialog>
#include "database_sig.hpp"

namespace dg {
	ImageDirWindow::ImageDirWindow(DatabaseSignal* sig, ImageDirModel* model, QWidget *const parent) :
		GeomRestore_Widget("ImageDirWindow", parent),
		_ui(new Ui::ImageDirWindow),
		_model(model)
	{
		_ui->setupUi(this);
		_ui->dirview->setModel(model);
		// ロードの進捗を示す
		connect(sig, &DatabaseSignal::processingDir,
				this, [this](const QString& path){
					_ui->lbProcessing->setText(path);
				});
	}
	void ImageDirWindow::addDir() {
		const QString path(QFileDialog::getExistingDirectory(this, tr("Select Image Directory"), nullptr));
		// wineにて無効なディレクトリが返ってくる時があるのでそれを弾く
		// (またはDontUseNativeDialogを指定する)
		if(QFileInfo::exists(path)) {
			if(!path.isEmpty()) {
				_ui->pbAdd->setEnabled(false);
				_ui->pbRemove->setEnabled(false);
				_model->addDir(path);
				_ui->pbAdd->setEnabled(true);
				_ui->pbRemove->setEnabled(true);
			}
		}
	}
	void ImageDirWindow::remDir() {
		// 単体選択時のみ対応
		const QModelIndexList sl = _ui->dirview->selectionModel()->selectedRows();
		if(sl.size() == 1)
			_model->removeDir(sl.first());
	}
}
