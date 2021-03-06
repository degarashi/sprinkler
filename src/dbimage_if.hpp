#pragma once
#include "idtype.hpp"
#include <QString>
#include <QByteArray>
#include <QSize>
#include <QPixmap>

class QAbstractItemModel;
class QAbstractItemDelegate;
namespace dg {
	//! SQLデータベースで扱う情報をファイルから読み取り
	struct ImageInfo {
		QString		fileName;
		QSize		size;
		QByteArray	hash;
		qint64		fileTime;		// MSecsSinceEpoch()
		int			candFlag;
		DirId		loadedDirId;
		struct CantLoad : std::runtime_error {
			using std::runtime_error::runtime_error;
		};

		ImageInfo() = default;
		ImageInfo(const QString& path);
		//! デフォルトコネクションのSQLデータベースに対し、画像を登録(新規)
		ImageId addEntry(DirId dirId) const;
		//! デフォルトコネクションのSQLデータベースに対し、既に登録された画像をアップデート
		void updateEntry(DirId dirId) const;

		int width() const;
		int height() const;
		int area() const;
		int aspect() const;

		static qint64 ToTime(const QString& f);
		static QByteArray ToHash(const QString& f);
		static bool CheckModified(qint64 now, qint64 cached);
		static bool CheckModified( const QByteArray& now, const QByteArray& cached);
	};
	struct ImageCand {
		ImageId		id;
		QSize		size;
	};
	using ImageCandV = std::vector<ImageCand>;

	class ImageQueryModel;
	using ImageModel_p = std::tuple<
		QAbstractItemModel*,
		ImageQueryModel*,
		QAbstractItemDelegate*
	>;
	class ColumnTarget;
	struct DBImage {
		virtual ImageModel_p makeImageModel(QObject* parent=nullptr) const = 0;
		virtual ImageInfo getImageInfo(ImageId id) const = 0;
		virtual QPixmap getThumbnail(ImageId id) const = 0;
		virtual const ColumnTarget* getImageColumnTarget() const = 0;
		// 使用された画像(cand_flag > 0)のフラグを全てリセット
		virtual void resetViewFlag() = 0;
		// タグ候補に該当する画像のフラグをリセット
		virtual void resetViewFlagSelected(const TagIdV& tag) = 0;
		//! タグのAND条件で画像検索し、そのIdリストを返す
		virtual ImageIdV findImageByTag(const TagIdV& tag) const = 0;
		// タグを持ち、cand_flag=0の画像からアスペクト比でソートした画像リストを作成
		// それをNBucket等分し、各Bucketの要素数が最大maxBucketになるように調整
		virtual ImageCandV enumImageByAspect(
					const TagIdV& tag,
					size_t nBucket,
					size_t maxBucket
				) = 0;
		// 候補に使用された画像にマーキング
		virtual void setViewFlag(const ImageIdV& img, int num) = 0;
		// 候補になったが使用されなかった画像のフラグをリセット
		virtual void resetSelectionFlag() = 0;
		virtual QString getFullPath(ImageId id) const = 0;
		// 画像に関連付けられたタグのリストを取得
		virtual TagIdV getTagFromImage(ImageId id, bool excludeDTag) const = 0;
		// 画像にタグをリンク付け(既にされていたら何もしない)
		virtual void makeTagLink(ImageId imgId, TagId tagId) = 0;
		// 画像からタグを外す(関連付けが無かったら何もしない)
		virtual void makeTagUnlink(ImageId imgId, TagId tagId) = 0;
	};
}
