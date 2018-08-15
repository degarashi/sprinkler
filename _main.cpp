/*
namespace dg {
	class Watcher;
	using WatchId = uint32_t;

	template <class Vec, class Val>
	decltype(*std::declval<Vec>().begin()) VectorAt(Vec& v, const Val& val) {
		const auto itr = std::find(v.begin(), v.end(), val);
		if(itr != v.end())
			return *itr;
		throw std::runtime_error("VectorAt: not found");
	}
}
*/
/*
#include <QApplication>
#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QHeaderView>
#include <QDebug>
#include <QFileDialog>
#include <QImageReader>
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDateTime>

constexpr char ConfigName[] = "config";
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QString conf = QStandardPaths::locate(QStandardPaths::AppConfigLocation, ConfigName);
    QString path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppConfigLocation);
    {
		QDir dir;
		dir.mkpath(path);
	}
    path += "/";
    path += ConfigName;

    QJsonObject obj;
    QDir dir(QFileDialog::getExistingDirectory(nullptr, "Select image directory", nullptr));
    auto files = dir.entryInfoList({"*.png", "*.jpg", "*.bmp", "*.gif"});
	for(const QFileInfo& f : files) {
		static int count =4;
		if(--count == 0)
			break;
//		QLabel* lb = new QLabel();
//		lb->setPixmap(QPixmap::fromImage(img));
//		lb->show();
		QFile file(f.absoluteFilePath());
		QDateTime dt = file.fileTime(QFile::FileModificationTime);
		QImage img(f.absoluteFilePath());
		QCryptographicHash hash(QCryptographicHash::Sha384);
		{
			QFile file(f.absoluteFilePath());
			file.open(QFile::ReadOnly);
			{
				auto data = file.readAll();
				hash.addData(data.data(), data.size());
			}
		}
		QByteArray hash64 = hash.result().toBase64();
		QJsonObject objent;
		objent["hash"] = QString(hash64);
		objent["mtime"] = dt.toString("hhAP");
		objent["hidden"] = true;
		obj[f.absoluteFilePath()] = objent;
	}
    QJsonDocument doc(obj);
	QFile file(path);
	file.open(QFile::ReadWrite);
    QByteArray jtex = doc.toJson();
    file.write(jtex.data(), jtex.size());
    return 0;

    QFileSystemModel fs;
    QObject::connect(&fs, &QFileSystemModel::rootPathChanged, [](QString str){
    	qDebug() << str;
    });
    fs.setReadOnly(true);
    fs.setRootPath(u8"");
    fs.setNameFilters({"*.png", "*.jpg", "*.tga"});
    fs.setNameFilterDisables(false);
    auto* tree = new QListView();
    tree->setModel(&fs);
    tree->show();
    tree->setRootIndex(fs.index(u8"/tmp"));
    tree->setWrapping(true);
    tree->setMovement(QListView::Free);
    tree->setViewMode(QListView::IconMode);
    tree->setResizeMode(QListView::Adjust);
    tree->setGridSize({100,100});
    tree->selectionModel();
    auto path = fs.filePath(tree->rootIndex());
    qDebug() << path;
    QObject::connect(tree, &QListView::doubleClicked, [&fs](QModelIndex index) { qDebug() << fs.fileName(index); });

    auto* t2 = new QTreeView();
    t2->setModel(&fs);
    t2->show();
    t2->setRootIndex(fs.index(u8"."));


//    QObject::connect(&fs, &QFileSystemModel::rowsInserted, [](QModelIndex idx, int i0, int i1){
//   		qDebug() << idx << i0 << i1; 
//    });
    return a.exec();
}
*/
