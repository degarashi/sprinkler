#include "imagedir_model.hpp"
#include "dbdir_if.hpp"

namespace dg {
	ImageDirModel::ImageDirModel(DBDir* dir, QObject* parent):
		QAbstractItemModel(parent),
		_dbDir(dir),
		_bReset(false)
	{}
	QModelIndex ImageDirModel::index(const int row, const int column, const QModelIndex &parent) const {
		if(_bReset)
			return QModelIndex();
		if(column >= Section::_Num)
			return QModelIndex();

		const auto& e = _getDirCache(ToDirId(parent));
		if(row >= int(e.child.size()))
			return QModelIndex();

		const auto idx = createIndex(
			row,
			column,
			e.child.at(row)
		);
		#ifdef REDUNDANT_CHECK
		{
			auto par = idx.parent();
			if(parent.isValid()) {
				par = createIndex(par.row(), parent.column(), parent.internalId());
			}
			Q_ASSERT(parent == par);
		}
		#endif
		return idx;
	}
	const QVariant& ImageDirModel::_getDataCache(const DataKey& key) const {
		auto itr = _dataCache.find(key);
		if(itr == _dataCache.end()) {
			QVariant d;
			if(key.dirId) {
				const auto info = _dbDir->getDirInfo(*key.dirId);
				switch(key.column) {
					case Section::Name:
						d = info.name;
						break;
					case Section::Path:
						d = info.path;
						break;
					case Section::ImageCount:
						d = QVariant::fromValue(_dbDir->numImagesInDir(*key.dirId));
						break;
					case Section::TotalCount:
					{
						const auto& e = _getDirCache(key.dirId);
						if(!e.child.empty())
							d = QVariant::fromValue(_dbDir->numTotalImagesInDir(*key.dirId));
						break;
					}
				}
			}
			itr = _dataCache.insert(key, d);
		}
		return itr.value();
	}
	const ImageDirModel::Ent& ImageDirModel::_getDirCache(const DirIdOpt id) const {
		const DirId did = id ? *id : -1;
		auto itr = _dirCache.find(did);
		if(itr == _dirCache.end()) {
			Ent ent;
			ent.child = _dbDir->getDirChild(id);
			if(id) {
				// (仮想ルートノード以外)
				ent.parent = _dbDir->getDirParent(*id);
				const auto& p_e = _getDirCache(ent.parent);
				ent.row = p_e.getChildPos(*id);
			} else
				ent.row = -1;
			itr = _dirCache.insert(did, std::move(ent));
		}
		return itr.value();
	}
	QModelIndex ImageDirModel::parent(const QModelIndex &child) const {
		if(_bReset)
			return QModelIndex();
		if(!child.isValid())
			return QModelIndex();

		const auto& e = _getDirCache(ToDirId(child));
		if(e.parent) {
			const auto& p_e = _getDirCache(e.parent);
			const auto parent = createIndex(
				p_e.row,
				0,
				*e.parent
			);
			#ifdef REDUNDANT_CHECK
			{
				const auto ch = _dbDir->getDirChild(ToDirId(parent));
				const auto itrC = std::find(ch.begin(), ch.end(), child.internalId());
				Q_ASSERT(itrC - ch.begin() == child.row());
			}
			#endif
			return parent;
		}
		// 親が存在しない
		return QModelIndex();
	}
	QVariant ImageDirModel::data(const QModelIndex &index, const int role) const {
		if(_bReset)
			return QVariant();
		if(!index.isValid())
			return QVariant();

		const int column = index.column();
		if(column >= Section::_Num)
			return QVariant();

		if(role == Qt::DisplayRole ||
			role == Qt::EditRole)
		{
			return _getDataCache(DataKey(index));
		}
		return QVariant();
	}
	Qt::ItemFlags ImageDirModel::flags(const QModelIndex &index) const {
		const auto& e = _getDirCache(ToDirId(index));
		auto f = QAbstractItemModel::flags(index);
		if(e.child.empty())
			f |= Qt::ItemNeverHasChildren;
		return f;
	}
	int ImageDirModel::rowCount(const QModelIndex &parent) const {
		if(_bReset)
			return 0;
		const auto& e = _getDirCache(ToDirId(parent));
		return e.child.size();
	}
	int ImageDirModel::columnCount(const QModelIndex &parent) const {
		Q_UNUSED(parent)
		return Section::_Num;
	}
	QVariant ImageDirModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
		if(role == Qt::DisplayRole) {
			if(section < Section::_Num) {
				const char* c_section[Section::_Num] = {
					"name",
					"path",
					"total count",
					"image count",
				};
				return c_section[section];
			}
		}
		return QAbstractItemModel::headerData(section, orientation, role);
	}
	void ImageDirModel::addDir(const QString &path) {
		_dbDir->addDir(path);
	}
	void ImageDirModel::removeDir(const QModelIndex &index) {
		const auto id = ToDirId(index);
		Q_ASSERT(id);
		_dbDir->removeDir(*id);
	}
	void ImageDirModel::beginReset() {
		_bReset = true;
		beginResetModel();
		_dataCache.clear();
		_dirCache.clear();
	}
	void ImageDirModel::endReset() {
		_bReset = false;
		endResetModel();
	}
}
