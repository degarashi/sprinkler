#pragma once
#include "geom_restore.hpp"
#include <memory>

namespace Ui {
	class DirList;
}
class QAbstractItemModel;
namespace dg {
	class DirList :
		public GeomRestore
	{
		Q_OBJECT
		public slots:
			// 選択中ディレクトリの追加削除
			void addDir();
			void remDir();
		private:
			std::shared_ptr<Ui::DirList>	_ui;
		public:
			explicit DirList(QAbstractItemModel* model, QWidget* parent=nullptr);
	};
}
