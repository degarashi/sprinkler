#pragma once
#include <QValidator>

namespace dg {
	class DBTag;
	class TagValidator :
		public QValidator
	{
		private:
			DBTag*		_dbTag;
		public:
			TagValidator(DBTag* tag, QObject* parent=nullptr);
			void fixup(QString& input) const override;
			State validate(QString& input, int& pos) const override;
	};
}
