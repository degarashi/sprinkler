#include "mainwindow.hpp"
#include <QApplication>
#include "dirlist.hpp"
#include <QTranslator>
#include <QTextCodec>
#include <QLibraryInfo>
#include <QDebug>

int main(int argc, char *argv[]) {
	// for XmbTextPropertyToTextList
	std::setlocale(LC_ALL, "");

	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("DegarashiLab");
	QCoreApplication::setOrganizationDomain("Degarashi.deg");
	QCoreApplication::setApplicationName("sprinkler");

	// Install Translation
	QTranslator tr;
	{
		QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
		QString loc = QLocale::system().name();

		tr.load("qtbase_" + loc,
				QLibraryInfo::location(QLibraryInfo::TranslationsPath)
			   );
		tr.load(QString("sprinkler_") + loc, QApplication::applicationDirPath());
		a.installTranslator(&tr);
	}

	dg::MainWindow mw;
	mw.show();
	return a.exec();
}
