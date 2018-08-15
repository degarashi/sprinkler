#include "mainwindow.hpp"
#include <QApplication>
#include "dirlist.hpp"

int main(int argc, char *argv[]) {
	// for XmbTextPropertyToTextList
	std::setlocale(LC_ALL, "");

	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("DegarashiLab");
	QCoreApplication::setOrganizationDomain("Degarashi.deg");
	QCoreApplication::setApplicationName("sprinkler");

	dg::MainWindow mw;
	mw.show();
	return a.exec();
}
