#pragma once
#include "idtype.hpp"
#include <QString>

class QObject;
namespace dg {
	struct DirInfo {
		QString		path,
					name;
	};
	class ImageDirModel;
	struct DBDir {
		// (ディレクトリ、タグ、画像)登録
		// 既に登録されていればそのIdを返す
		virtual void addDir(const QString& path) = 0;
		// (ディレクトリ、タグ、画像)削除
		virtual void removeDir(DirId id) = 0;
		// データの順序はアルファベットの降順
		virtual DirIdV getDirChild(DirIdOpt id) const = 0;
		virtual DirIdOpt getDirParent(DirId id) const = 0;
		virtual DirInfo getDirInfo(DirId id) const = 0;
		virtual DirIdV getRootDir() const = 0;
		virtual ImageDirModel* makeDirModel(QObject* parent=nullptr) = 0;
		virtual size_t numImagesInDir(DirId id) const = 0;
		virtual size_t numTotalImagesInDir(DirId id) const = 0;
	};
}
