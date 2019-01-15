#pragma once
#include "widget/gr_widget.hpp"
#include <memory>

namespace Ui {
	class ImageDirWindow;
}
namespace dg {
	class DatabaseSignal;
	class ImageDirModel;
	class ImageDirWindow :
		public widget::GeomRestore_Widget
	{
		Q_OBJECT
		public slots:
			// 選択中ディレクトリの追加削除
			void addDir();
			void remDir();
		private:
			std::shared_ptr<Ui::ImageDirWindow>	_ui;
			ImageDirModel*	_model;
		public:
			ImageDirWindow(DatabaseSignal* sig, ImageDirModel* model, QWidget* parent=nullptr);
	};
}
