#pragma once
#include "idtype.hpp"
#include <set>
#include <QAbstractListModel>

namespace dg {
	class DBTag;
	class DatabaseSignal;
	class TagSelectModel :
		public QAbstractListModel
	{
		Q_OBJECT
		private:
			using IdSet = std::set<TagId>;

			DBTag*		_dbTag;
			IdSet		_idSet;
		public:
			TagSelectModel(DBTag* tag,
							DatabaseSignal* sig,
							QObject* parent = nullptr);
			void add(TagId id);
			void rem(TagId id);
			TagIdV getArray() const;

			int rowCount(const QModelIndex &parent) const override;
			QVariant data(const QModelIndex &index, int role) const override;
			Qt::ItemFlags flags(const QModelIndex &index) const override;
	};
}
