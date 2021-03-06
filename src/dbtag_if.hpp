#pragma once
#include "idtype.hpp"
#include <QString>

class QObject;
class QSqlTableModel;
namespace dg {
	struct DBTag {
		struct CountImage {
			size_t	total,
					shown;

			size_t notshown() const noexcept {
				Q_ASSERT(total >= shown);
				return total - shown;
			}
		};
		//! イメージ総数, 表示済みフラグ数
		virtual CountImage countImageByTag(const TagIdV& tag) const = 0;
		//! もう存在しないタグを除外
		virtual TagIdV excludeRemovedTag(const TagIdV& tag) const = 0;
		virtual TagIdOpt getTagId(const QString& name) const = 0;
		virtual QString getTagName(TagId id) const = 0;
		virtual TagIdV enumTagForwardMatch(const QString& str) const = 0;
		virtual QSqlTableModel* makeTagModel(QObject* parent=nullptr) const = 0;
		//! タグ検索クエリ文
		virtual QString tagMatchQuery(QStringList getcol, const TagIdV& tag, bool emptyIsAll) const = 0;
		virtual void markAsUsedRecentry(const TagIdV& tag) = 0;
		virtual TagIdV getRecentryUsed(size_t limit, bool notZero) const = 0;
		virtual void resetMRU() = 0;
		// 既にタグが存在すればそれを返す
		virtual TagId makeTag(const QString& name) = 0;
		//! (内部用)タグの孤立チェック
		virtual bool isIsolatedTag(TagId tag) const = 0;
	};
}
