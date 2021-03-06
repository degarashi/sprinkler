#pragma once
#include "dbtag_if.hpp"
#include "dbdir_if.hpp"
#include "dbimage_if.hpp"
#include "database_sig.hpp"
#include "spine/src/enum.hpp"
#include <functional>
#include <QSqlDatabase>

class QFileInfo;
class QSqlDatabase;
class QSqlTableModel;
namespace dg {
	class ImageQueryModel;
	class ImageDirModel;
	class Database :
		public DatabaseSignal,
		public DBImage,
		public DBDir,
		public DBTag
	{
		Q_OBJECT
		private:
			//! delフラグが有効ならデストラクタでQSqlDatabase(デフォルトコネクション)を削除
			struct RemoveConnection {
				bool	del = false;
				~RemoveConnection();
			};
			//! [Dir, Tag, Image]の三種の変更フラグを持ち、デストラクタでそれをシグナル通知
			class ResetSignal {
				//! 内部にフラグを持ち、最後に変更を返り値で示す
				class ChangeFlag {
					private:
						bool	_changed;
					public:
						ChangeFlag();
						bool change();
						bool end();
				};
				private:
					ChangeFlag	_dir,
								_tag,
								_image;
					Database*	_db;
				public:
					ResetSignal(Database* db);
					~ResetSignal();
					void dirChange();
					void imageChange();
					void tagChange();
			};
			DefineEnum(
				CheckResult,
				(NoUpdate)
				(Update)
				(Deleted)
			);

			RemoveConnection	_remc;
			ColumnTarget*		_ct;
			QSqlDatabase		_db;
			//! 何らかの処理中に別の処理関数が呼ばれたらアサート掛ける為のフラグ
			mutable bool		_bProcessing;
			using ThumbnailCache = QHash<ImageId, QPixmap>;
			mutable ThumbnailCache	_thumbnail;

			// 親を持っていないノードを列挙
			DirId _getRootDir(DirId id) const;

			using CBCollectImage = std::function<bool (const QFileInfo&)>;
			// ディレクトリ内の画像ファイルを列挙し、コールバック関数に渡す
			// trueを返したら読み込み対象とみなしデータベースに組み込む
			void _collectImageInDir(ResetSignal& sig, const QString& path, DirId dirId, const CBCollectImage& cb);

			// _addDirから呼ばれる
			void _addDir_Setup(const QString& path);
			// フルパスと親ディレクトリIDを指定し、
			// ディレクトリとそこに含まれる画像群をSQLデータベースへ登録
			void _addDir_Rec(ResetSignal& sig, const QString& path, std::optional<DirId> parentId);
			// ImageInfoクラスを使ってSQLデータベースに画像を登録
			// ファイルにアクセスできない場合はnulloptを返す
			std::optional<ImageId> _addImage(DirId dirId, const QString& path);

			// ディレクトリとそこに登録された画像、タグを削除
			// 末端ノードを指定しないと後のvalidationに失敗する
			void _removeDirPrivate(DirId id, bool upToRoot);
			// ディレクトリとそこに登録された画像、タグを削除
			// _removeDirPrivateから呼ばれる
			// (末端ノードのみ対応。assert対象)
			void _removeDirSingle(ResetSignal& sig, DirId id);
			// ディレクトリIDから親に向かって走査した時のIDリスト(自分を含む)
			DirIdV _enumAncestor(DirId id) const;
			// ディレクトリID -> フルパス　の変換
			QString _getFullPath(DirId id) const;

			// ------------ サムネイル ------------
			// サムネイルファイルが(あったら)削除
			static void _RemoveThumbnailFile(ImageId id);
			// メモリ上、ファイルとして保存してあるサムネイルを削除
			void _removeThumbnail(ImageId id);
			// DirIdに関連するImageのサムネイルを削除
			void _removeThumbnailInDir(DirId id);
			//! サムネイルディレクトリを全消し
			void _clearThumbnail();
			// メモリ上にあるサムネイルをファイルに保存
			void _saveThumbnailAsFile();

			static QPixmap _LoadThumbnailFromFile(ImageId id);
			using CBThumbnail = std::function<void (const QFileInfo&)>;
			static void _EnumThumbnailFile(const CBThumbnail& cb);
			static QString _CacheLocation(bool create);
			static QString _ConfigLocation(bool create);
			static QString _ThumbnailLocation(bool create);
			static QString _ThumbnailFileName(ImageId id);

			// ------------ サブディレクトリ列挙 ------------
			static QStringList _ImageFilter();
			using CBEnumSubDir = CBThumbnail;
			static void _EnumSubDir(
				const QString &path,
				const CBEnumSubDir &cb,
				bool noSymLink
			);
			using CBEnumImageDir = CBThumbnail;
			static void _EnumImage(const QString& path, const CBEnumImageDir& cb);

			DirIdOpt _dirFromPath(const QString& path) const;

			// ------------ 整合性チェック ------------
			void _validation() const;
			void _imageDirValidation(const QString& path) const;
			CheckResult _checkImage(ImageId id, bool forceHashCheck);

			// ------------ 初期化 ------------
			void _init(bool forceClear);
			QSqlDatabase _makeConnection();
			bool _initDatabase(bool createFile);
			void _createTable();
			// キャッシュに残っているデータと今のファイルシステムとで整合性を持たせる
			// 無効なデータを消し、新しいデータは登録する
			void _updateDatabase();
			// ディレクトリ内の画像ファイル列挙 & 登録(再帰)
			void _updateDatabase_Rec(const DirId id, const QString& path);
			// キャッシュデータの整合性を判定
			// \return キャッシュが壊れている場合はfalse
			bool _checkDatabaseValidness();

			// ------------ バージョン管理 ------------
			bool _checkAppVersion() const;
			void _writeAppVersion() const;

		public:
			static struct _tagInit {} TagInit;
			// データベースを全クリア
			Database(_tagInit, QObject* parent=nullptr);
			// 既存のデータベースを読み込み。更新チェック
			explicit Database(QObject* parent=nullptr);
			~Database();

			// ------------- from DBTag -------------
			TagIdV excludeRemovedTag(const TagIdV& tag) const override;
			CountImage countImageByTag(const TagIdV& tag) const override;
			TagIdOpt getTagId(const QString& name) const override;
			QString getTagName(TagId id) const override;
			TagIdV enumTagForwardMatch(const QString& str) const override;
			QSqlTableModel* makeTagModel(QObject* parent=nullptr) const override;
			QString tagMatchQuery(QStringList getcol, const TagIdV& tag, bool emptyIsAll) const override;
			void markAsUsedRecentry(const TagIdV& tag) override;
			TagIdV getRecentryUsed(size_t limit, bool notZero) const override;
			void resetMRU() override;
			TagId makeTag(const QString& name) override;
			bool isIsolatedTag(TagId tag) const override;

			// ------------- from DBImage -------------
			ImageModel_p makeImageModel(QObject* parent=nullptr) const override;
			ImageInfo getImageInfo(ImageId id) const override;
			QPixmap getThumbnail(ImageId id) const override;
			const ColumnTarget* getImageColumnTarget() const override;
			void resetViewFlag() override;
			void resetViewFlagSelected(const TagIdV& tag) override;
			ImageIdV findImageByTag(const TagIdV& tag) const override;
			ImageCandV enumImageByAspect(
							const TagIdV& tag,
							size_t nBucket,
							size_t maxBucket
						) override;
			void setViewFlag(const ImageIdV& img, int num) override;
			// 候補には挙がったが使用されなかった、または
			// アプリの強制終了などで中途半端になったフラグをリセット
			void resetSelectionFlag() override;
			QString getFullPath(ImageId id) const override;
			TagIdV getTagFromImage(ImageId id, bool excludeDTag) const override;
			void makeTagLink(ImageId imgId, TagId tagId) override;
			void makeTagUnlink(ImageId imgId, TagId tagId) override;

			// ------------- from DBDir -------------
			void addDir(const QString& path) override;
			void removeDir(DirId id) override;
			DirIdV getDirChild(DirIdOpt id) const override;
			DirIdOpt getDirParent(DirId id) const override;
			DirInfo getDirInfo(DirId id) const override;
			DirIdV getRootDir() const override;
			ImageDirModel* makeDirModel(QObject* parent=nullptr) override;
			size_t numImagesInDir(DirId id) const override;
			size_t numTotalImagesInDir(DirId id) const override;
	};
}
