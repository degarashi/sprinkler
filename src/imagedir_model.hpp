#pragma once
#include "idtype.hpp"
#include "spine/src/enum.hpp"
#include <QAbstractItemModel>

namespace dg {
	class DBDir;
	// データの順序はアルファベットの降順
	class ImageDirModel :
		public QAbstractItemModel
	{
		Q_OBJECT

		public:
			struct DataKey {
				DirIdOpt	dirId;
				int			column;

				DataKey(const QModelIndex& idx) {
					if((dirId = ToDirId(idx)))
						column = idx.column();
					else
						column = -1;
				}
				bool operator == (const DataKey& k) const noexcept {
					return dirId == k.dirId &&
							column == k.column;
				}
				uint hashValue(const uint seed) const {
					return qHash(dirId ? *dirId : -1, seed) ^ qHash(column);
				}
			};
		private:
			struct Ent {
				DirIdOpt		parent;
				// 親からの位置
				int				row;
				DirIdV			child;

				int getChildPos(const DirId id) const {
					// 親ノードの中でidが何番目かを特定
					const auto itr = std::find(child.begin(), child.end(), id);
					Q_ASSERT(itr != child.end());
					return int(itr - child.begin());
				}
			};
			using DirCache = QHash<DirId, Ent>;
			mutable DirCache	_dirCache;

			static DirIdOpt ToDirId(const QModelIndex& idx) {
				if(idx.isValid())
					return idx.internalId();
				return std::nullopt;
			}
			using DataCache = QHash<DataKey, QVariant>;
			mutable DataCache	_dataCache;

			const Ent& _getDirCache(DirIdOpt id) const;
			const QVariant& _getDataCache(const DataKey& key) const;
			int _rowCount(const QModelIndex &parent) const;

			DBDir*	_dbDir;

			DefineEnum(
				Section,
				(Name)
				(Path)
				(TotalCount)
				(ImageCount)
			);

		public slots:
			void beginReset();
			void endReset();
		public:
			ImageDirModel(DBDir* dir, QObject* parent=nullptr);
			void addDir(const QString& path);
			void removeDir(const QModelIndex& index);

			// ---- Model Interface ----
			QModelIndex index(int row, int column, const QModelIndex &parent) const override;
			QModelIndex parent(const QModelIndex &child) const override;
			int rowCount(const QModelIndex &parent) const override;
			int columnCount(const QModelIndex &parent) const override;
			QVariant data(const QModelIndex &index, int role) const override;
			Qt::ItemFlags flags(const QModelIndex &index) const override;
			QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	};
	inline uint qHash(const ImageDirModel::DataKey& k, const uint seed) noexcept {
		return k.hashValue(seed);
	}
}
