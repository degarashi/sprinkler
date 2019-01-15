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
			void beginResetLink();

		 	void endResetDir();
			void endResetImage();
			void endResetTag();
			void endResetLink();

			void processingDir(const QString& path);
		public:
			explicit DatabaseSignal(QObject* parent=nullptr);
	};
}
