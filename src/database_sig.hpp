#pragma once
#include <QObject>

namespace dg {
	class DatabaseSignal :
		public QObject
	{
		Q_OBJECT

		signals:
			void beginResetDir();
			void beginResetImage();
			void beginResetTag();

		 	void endResetDir();
			void endResetImage();
			void endResetTag();
		public:
			explicit DatabaseSignal(QObject* parent=nullptr);
	};
}
