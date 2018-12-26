#include "imagequery_model.hpp"
#include "table_desc.hpp"
#include "dbimage_if.hpp"
#include "dbtag_if.hpp"

/* To Writable Model(Signal)
	dataChanged
	headerDataChanged
	beforeUpdate
	beforeInsert
	beforeDelete
	primeInsert

	rowsAboutToBeInserted
	rowsAboutToBeRemoved
	rowsAboutToBeMoved
	columnsAboutToBeInserted
	columnsAboutToBeRemoved
	columnsAboutToBeMoved
	modelAboutToBeReset

	rowsInserted
	rowsRemoved
	rowsMoved
	columnsInserted
	columnsRemoved
	columnsMoved
	modelReset
*/
namespace dg {
	namespace {
		constexpr const char* c_headerName[] = {
			"name",
			"width",
			"height",
			"aspect",
			"cand_flag"
		};
	}
	ImageQueryModel::ImageQueryModel(const DBImage* img, const DBTag* tag, QObject* parent):
		base_t(parent),
		_dbImg(img),
		_dbTag(tag),
		_sub(new sub_t(img->getImageColumnTarget(), this)),
		_ct(new ColumnTarget(this))
	{
		// (チェックボックス表示したいカラムがあればここで指定)
		connect(_sub, &QAbstractItemModel::modelAboutToBeReset,
				this, [this](){ beginResetModel(); });
		connect(_sub, &QAbstractItemModel::modelReset,
				this, [this](){ endResetModel(); });
	}
	ColumnTarget* ImageQueryModel::getColumnTarget() const noexcept {
		return _ct;
	}
	void ImageQueryModel::setTable(const QString& tableName) {
		_tableName = tableName;
	}
	int ImageQueryModel::rowCount(const QModelIndex&) const {
		return _sub->rowCount();
	}
	int ImageQueryModel::columnCount(const QModelIndex& parent) const {
		Q_UNUSED(parent)
		return Column::_Num;
	}
	Qt::ItemFlags ImageQueryModel::flags(const QModelIndex &index) const {
		Qt::ItemFlags flag = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		if(index.isValid() && _ct->has(index.column()))
			flag |= Qt::ItemIsUserCheckable;
		return flag;
	}
	QVariant ImageQueryModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
		if(orientation == Qt::Horizontal &&
			section < Column::_Num &&
			role == Qt::DisplayRole)
			return c_headerName[section];
		return QVariant();
	}
	void ImageQueryModel::setTagFilter(const TagIdV& tag) {
		_tag = tag;
		select();
	}
	void ImageQueryModel::beginReset() {
		_sub->setQuery("SELECT 1 WHERE False");
		_tagBk = _tag;
		_tag.clear();
	}
	void ImageQueryModel::endReset() {
		// 削除されたタグを精査
		_tag = _dbTag->excludeRemovedTag(_tagBk);
		_tagBk.clear();
		// リスト更新
		select();
	}
	bool ImageQueryModel::select() {
		const QStringList getcol{Img_id, Img_file_name, Img_dir_id, Img_width, Img_height, Img_area, Img_aspect, Img_hash, Img_modify_date, Img_cand_flag};
		_sub->setQuery(_dbTag->tagMatchQuery(getcol, _tag, false));
		return true;
	}
	QVariant ImageQueryModel::data(const QModelIndex &idx, const int role) const {
		if(idx.isValid()) {
			const auto getValue = [row=idx.row(), role, this](const auto idxSrc){
				return _sub->data(_sub->index(row, idxSrc), role);
			};
			switch(idx.column()) {
				case Column::IconAndName:
					if(role == Qt::DecorationRole) {
						// サムネイル画像を返す
						const auto idx2 = createIndex(idx.row(), Image_Column::Id);
						const ImageId id = _sub->data(idx2, Qt::DisplayRole).toInt();
						return _dbImg->getThumbnail(id);
					}
					return getValue(Image_Column::FileName);

				// インデックス変換の後、対応する値を返す
				case Column::Width:
					return getValue(Image_Column::Width);
				case Column::Height:
					return getValue(Image_Column::Height);
				case Column::Aspect:
					return getValue(Image_Column::Aspect);
				case Column::CandFlag:
					return getValue(Image_Column::CandFlag);
			}
		}
		return QVariant();
	}
}
