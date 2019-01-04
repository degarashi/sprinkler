#include "sprinkler.hpp"
#include <QApplication>
#include <QTranslator>
#include <QTextCodec>
#include <QLibraryInfo>
#include <QSettings>

int main(int argc, char *argv[]) {
	// for XmbTextPropertyToTextList(for Linux)
	std::setlocale(LC_ALL, "");

	QApplication a(argc, argv);

	// set organization name etc...
	QCoreApplication::setOrganizationName("DegarashiLab");
	QCoreApplication::setOrganizationDomain("Degarashi.deg");
	QCoreApplication::setApplicationName("sprinkler");

	// set default settings file format (INI)
	QSettings::setDefaultFormat(QSettings::IniFormat);

	// install translation
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

	const dg::Sprinkler spr;
	return a.exec();
}
