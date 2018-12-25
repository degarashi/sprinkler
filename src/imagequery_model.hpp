#pragma once
#include "checkbox_model.hpp"
#include "idtype.hpp"
#include <QSqlQueryModel>

namespace dg {
	class DBImage;
	class DBTag;
	// 読み取り専用
	class ImageQueryModel:
		public QAbstractTableModel
	{
		Q_OBJECT
		public:
			struct Column {
				enum {
					IconAndName,
					Width,
					Height,
					Aspect,
					CandFlag,
					_Num
				};
			};
		private:
			using base_t = QAbstractTableModel;
			using sub_t = CheckBoxModel<QSqlQueryModel>;

			QModelIndex _toBaseIndex(const QModelIndex& idx) const;
			const DBImage*		_dbImg;
			const DBTag*		_dbTag;
			sub_t*				_sub;
			ColumnTarget*		_ct;
			QString				_tableName;
			TagIdV				_tag,
								_tagBk;			//!< モデルリセット中のデータ退避用

		public slots:
			void beginReset();
			void endReset();

		public:
			ImageQueryModel(const DBImage* img, const DBTag* tag, QObject* parent=nullptr);

			int rowCount(const QModelIndex &parent) const override;
			int columnCount(const QModelIndex &parent) const override;
			QVariant data(const QModelIndex &idx, int role=Qt::DisplayRole) const override;
			Qt::ItemFlags flags(const QModelIndex &index) const override;
			QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;

			ColumnTarget* getColumnTarget() const noexcept;
			// For Sub
			void setTable(const QString& tableName);
			bool select();

			void setTagFilter(const TagIdV& tag);
	};
}
