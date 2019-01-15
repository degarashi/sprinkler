#include "dbimage_if.hpp"
#include "sql/insert.hpp"
#include "table_desc.hpp"
#include <QImageReader>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QDateTime>

namespace dg {
	ImageInfo::ImageInfo(const QString& path) {
		const QImageReader reader(path);
		if(!reader.canRead())
			throw ImageInfo::CantLoad(path.toStdString());

		size = reader.size();
		const QFileInfo f(path);
		fileName = f.fileName();
		hash = ToHash(f.absoluteFilePath());
		fileTime = ToTime(path);
		candFlag = 0;
		loadedDirId = -1;
	}
	void ImageInfo::updateEntry(const DirId dId) const {
		const auto q = sql::Query(
			"UPDATE " Image_Table " SET\n"
			Img_width		"=?,\n"
			Img_height		"=?,\n"
			Img_area		"=?,\n"
			Img_aspect		"=?,\n"
			Img_hash		"=?,\n"
			Img_modify_date	"=?\n"
			"WHERE " Img_dir_id "=? AND " Img_file_name "=?",
			width(),
			height(),
			area(),
			aspect(),
			hash,
			fileTime,
			dId, fileName
		);
		Q_ASSERT(q.numRowsAffected() == 1);
	}
	ImageId ImageInfo::addEntry(const DirId dId) const {
		Q_ASSERT(dId >= 0);
		sql::InsertInto(
			Image_Table,
			Img_file_name,		fileName,
			Img_dir_id,			dId,
			Img_width,			width(),
			Img_height,			height(),
			Img_area,			area(),
			Img_aspect,			aspect(),
			Img_hash,			hash,
			Img_modify_date,	fileTime
		);
		return sql::GetMaxId(Image_Table);
	}
	int ImageInfo::width() const {
		return size.width();
	}
	int ImageInfo::height() const {
		return size.height();
	}
	int ImageInfo::area() const {
		return width() * height();
	}
	int ImageInfo::aspect() const {
		return int(width() * 100 / height()) / 10;
	}
	qint64 ImageInfo::ToTime(const QString& f) {
		return QFileInfo(f).fileTime(QFile::FileModificationTime).toMSecsSinceEpoch();
	}
	QByteArray ImageInfo::ToHash(const QString& f) {
		QFile file(f);
		if(!file.open(QIODevice::ReadOnly))
			throw ImageInfo::CantLoad(f.toStdString());

		// [ファイルサイズ] + [先頭 NBytes] + [中間 NBytes] + [末尾 NBytes] = Hash
		QCryptographicHash h(QCryptographicHash::Md5);
		const qint64 size = file.size();
		h.addData(reinterpret_cast<const char*>(&size), sizeof(size));
		constexpr const qint64 WindowSize = 85;
		char buff[WindowSize];
		// 先頭
		file.seek(0);
		h.addData(buff, file.read(buff, std::min(WindowSize, size)));
		// 中間
		file.seek(size/2);
		h.addData(buff, file.read(buff, std::min(WindowSize, size/2)));
		// 末尾
		file.seek(std::max<qint64>(0, size-WindowSize));
		h.addData(buff, file.read(buff, size-file.pos()));
		return h.result();
	}
	bool ImageInfo::CheckModified(const qint64 now, const qint64 cached) {
		return cached < now;
	}
	bool ImageInfo::CheckModified(const QByteArray& now, const QByteArray& cached) {
		return now != cached;
	}
}
