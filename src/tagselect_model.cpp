#include "tagselect_model.hpp"
#include "dbtag_if.hpp"
#include "database_sig.hpp"

namespace dg {
	TagSelectModel::TagSelectModel(DBTag* tag, DatabaseSignal* sig, QObject* parent):
		QAbstractListModel(parent),
		_dbTag(tag)
	{
		connect(sig, &DatabaseSignal::beginResetTag,
				this, [this](){ beginResetModel(); });
		connect(sig, &DatabaseSignal::endResetTag,
				this, [this](){
					// 存在しないタグ番号を削除
					TagIdV tag(_idSet.begin(), _idSet.end());
					tag = _dbTag->excludeRemovedTag(tag);
					_idSet.clear();
					for(auto id : tag)
						_idSet.emplace(id);
					endResetModel();
				});
	}
	void TagSelectModel::add(const TagId id) {
		beginResetModel();
		_idSet.emplace(id);
		endResetModel();
	}
	void TagSelectModel::rem(const TagId id) {
		Q_ASSERT(id < TagId(_idSet.size()));
		auto itr = _idSet.begin();
		std::advance(itr, id);

		beginResetModel();
		_idSet.erase(itr);
		endResetModel();
	}
	int TagSelectModel::rowCount(const QModelIndex& parent) const {
		if(parent.isValid())
			return 0;
		return int(_idSet.size());
	}
	QVariant TagSelectModel::data(const QModelIndex& index, const int role) const {
		if(!index.parent().isValid()) {
			if(role == Qt::DisplayRole ||
				role == Qt::EditRole)
			{
				auto itr = _idSet.begin();
				std::advance(itr, index.row());
				if(role == Qt::DisplayRole) {
					const QString tagName = _dbTag->getTagName(*itr);
					Q_ASSERT(!tagName.isNull());
					return tagName;
				}
				return *itr;
			}
		}
		return QVariant();
	}
	Qt::ItemFlags TagSelectModel::flags(const QModelIndex&) const {
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	}
	TagIdV TagSelectModel::getArray() const {
		TagIdV ret;
		for(auto& e : _idSet)
			ret.emplace_back(e);
		return ret;
	}
}
