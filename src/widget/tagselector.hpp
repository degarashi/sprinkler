#pragma once
#include "../idtype.hpp"
#include <QWidget>
#include <memory>

namespace Ui {
	class TagSelector;
}
class QSqlTableModel;
namespace dg {
	class TagSelectModel;
	class DBTag;
	class DatabaseSignal;
	namespace widget {
		class TagSelector :
			public QWidget
		{
			Q_OBJECT
			private:
				using UI = std::shared_ptr<::Ui::TagSelector>;
				UI					_ui;
				DBTag				*_dbTag;
				TagSelectModel		*_tagSelected;		//!< 選択されたタグ
				QSqlTableModel		*_tagCand;

				void _refreshCount() const;

			public:
				TagSelector(QWidget* parent=nullptr);
				void init(DBTag* tag, DatabaseSignal* sig);
				TagIdV getArray() const;
			public slots:
				void onAdd();
				void onRem();
			signals:
				// タグの選択状態が変更された
				void changed(const TagIdV& tag);
		};
	}
}
