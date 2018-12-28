#pragma once
#include <QValidator>

namespace dg {
	class DBTag;
	class TagValidator :
		public QValidator
	{
		private:
			const DBTag*		_dbTag;
		public:
			TagValidator(const DBTag* tag, QObject* parent=nullptr);
			void fixup(QString& input) const override;
			State validate(QString& input, int& pos) const override;
	};
}
