#pragma once
#include "widget/gr_widget.hpp"
#include <memory>

namespace Ui {
	class ImageTagWindow;
}
class QSqlTableModel;
namespace dg {
	class TagSelectModel;
	class DBImage;
	class DBTag;
	class DatabaseSignal;
	class ImageQueryModel;
	class ImageTagWindow :
		public widget::GeomRestore_Widget
	{
		Q_OBJECT

		public slots:
			void onSearch();
		private:
			using UI = std::shared_ptr<Ui::ImageTagWindow>;
			UI					_ui;
			ImageQueryModel		*_result;			//!< 検索結果
			// SearchボタンのEnable/Disable用
			bool				_dirty;

			bool _setDirty();
			bool _resetDirty();
		public:
			ImageTagWindow(
				DBTag* tag,
				DBImage* img,
				DatabaseSignal* sig,
				QWidget *parent=nullptr
			);
	};
}
