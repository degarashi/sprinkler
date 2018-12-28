#pragma once
#include "idtype.hpp"
#include <QDialog>
#include <memory>

namespace Ui {
	class TagInput;
}

namespace dg {
	class DatabaseSignal;
	class DBTag;
	class TagInput :
		public QDialog
	{
		Q_OBJECT
		private:
			std::shared_ptr<Ui::TagInput>	_ui;
			void _setExistingMode(bool b);
			void _setOKEnabled(bool b);
			bool _getExistingMode() const;
		public:
			TagInput(DatabaseSignal* sig,
					DBTag* dbTag,
					QWidget *parent=nullptr);
		signals:
			void accepted(const TagIdV& tag);
	};
}
