#pragma once
#include "idtype.hpp"
#include <QString>

class QObject;
class QSqlTableModel;
namespace dg {
	struct DBTag {
		//! イメージ総数, 表示済みフラグ数
		virtual std::pair<size_t,size_t> countImageByTag(const TagIdV& tag) const = 0;
		//! もう存在しないタグを除外
		virtual TagIdV excludeRemovedTag(const TagIdV& tag) const = 0;
		virtual TagIdOpt getTagId(const QString& name) const = 0;
		virtual QString getTagName(TagId id) const = 0;
		virtual TagIdV enumTagForwardMatch(const QString& str) const = 0;
		virtual QSqlTableModel* makeTagModel(QObject* parent=nullptr) const = 0;
		//! タグ検索クエリ文
		virtual QString tagMatchQuery(QStringList getcol, const TagIdV& tag, bool emptyIsAll) const = 0;
		virtual void markAsUsedRecentry(const TagIdV& tag) = 0;
		virtual TagIdV getRecentryUsed(size_t limit) const = 0;
		virtual void resetMRU() = 0;
	};
}
