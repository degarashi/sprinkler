#include "database.hpp"
#include "aux.hpp"
#include "sql/value.hpp"
#include "sql/transaction.hpp"
#include "sql/insert.hpp"
#include "imagedir_model.hpp"
#include "table_desc.hpp"
#include "imagequery_model.hpp"
#include "checkbox_proxy.hpp"
#include "checkbox_delegate.hpp"
#include "version.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QFileInfo>
#include <QImageReader>
#include <QSqlDriver>
#include <QStringList>
#include <QSqlTableModel>
#include <QPixmap>
#include <QSettings>
#include <cmath>

namespace dg {
	// -------------- RemoveConnection --------------
	Database::RemoveConnection::~RemoveConnection() {
		if(del) {
			if(std::uncaught_exceptions() == 0)
				QSqlDatabase::removeDatabase(QSqlDatabase::connectionNames().at(0));
		}
	}
	// -------------- DatabaseSignal --------------
	DatabaseSignal::DatabaseSignal(QObject *const parent):
		QObject(parent)
	{}
	// -------------- Database --------------
	namespace {
		const QString c_databaseFile(QStringLiteral("database.sqlite")),
						c_dbType(QStringLiteral("QSQLITE")),
						c_thumbnailDir(QStringLiteral("thumbnail"));
	}
	Database::CheckResult Database::_checkImage(const ImageId id, const bool forceHashCheck) {
		auto q = sql::Query(
			"SELECT " Img_modify_date ", " Img_hash " FROM " Image_Table " WHERE " Img_id "=?",
			id
		);
		const auto path = getFullPath(id);
		if(!QFileInfo::exists(path)) {
			// ファイルが削除された

			// TagILinkを削除
			sql::Query(
				"DELETE FROM " TagILink_Table " WHERE " TIL_image_id "=?",
				id
			);
			// 孤立Tagを削除
			{
				auto q = sql::Query(
					"SELECT tag." Tag_id " FROM " Tag_Table " tag\n"
					"	OUTER LEFT JOIN " TagDLink_Table " dl\n"
					"		ON tag." Tag_id "=dl." TDL_tag_id "\n"
					"	WHERE dl." TDL_tag_id " IS NULL"
				);
				if(q.next()) {
					const auto q2 = sql::Query(
						"DELETE FROM " Tag_Table " WHERE " Tag_id "=?",
						sql::GetRequiredValue<TagId>(q, 0, false)
					);
					Q_ASSERT(q2.numRowsAffected()==1);
				}
			}
			// Imageエントリを削除
			const auto q = sql::Query(
				"DELETE FROM " Image_Table " WHERE " Img_id "=?",
				id
			);
			Q_ASSERT(q.numRowsAffected()==1);
			return CheckResult::Deleted;
		} else {
			// 整合性チェック(更新時刻)
			if(forceHashCheck ||
				ImageInfo::CheckModified(
					ImageInfo::ToTime(path),
					sql::GetRequiredValue<qint64>(q, 0, true)
				)
			) {
				// 整合性チェック(ハッシュ)
				if(ImageInfo::CheckModified(
					ImageInfo::ToHash(path),
					sql::GetRequiredValue<QByteArray>(q, 1, false))
				) {
					// データ更新
					ImageInfo(path).updateEntry(id);
					return CheckResult::Update;
				}
				// 時刻だけ更新
				const auto q = sql::Query(
					"UPDATE " Image_Table " SET " Img_modify_date "=? WHERE " Img_id "=?",
					ImageInfo::ToTime(path),
					id
				);
				Q_ASSERT(q.numRowsAffected()==1);
			}
		}
		return CheckResult::NoUpdate;
	}
	void Database::_createTable() {
		sql::Query(
			"CREATE TABLE " ImageDir_Table " (\n"
			IDir_id			" INTEGER PRIMARY KEY,\n"
			IDir_name		" TEXT NOT NULL,\n"
			IDir_path		" TEXT,\n"
			IDir_parent_id	" INTEGER,\n"
			"CHECK((path IS NULL) <> (parent_id IS NULL)),\n"
			"UNIQUE(" IDir_parent_id ", " IDir_name "),\n"
			"FOREIGN KEY(" IDir_parent_id ") REFERENCES " ImageDir_Table "(" IDir_id ") \n"
			")"
		);
		sql::Query(
			"CREATE TABLE " Image_Table " (\n"
			Img_id			" INTEGER PRIMARY KEY,\n"
			Img_file_name	" TEXT NOT NULL,\n"
			Img_dir_id		" INTEGER NOT NULL,\n"
			Img_width		" INTEGER NOT NULL,\n"
			Img_height		" INTEGER NOT NULL,\n"
			Img_area		" INTEGER NOT NULL,\n"
			Img_aspect		" INTEGER NOT NULL,\n"
			Img_hash		" BLOB NOT NULL,\n"
			Img_modify_date	" INTEGER NOT NULL,\n"
			Img_cand_flag	" INTEGER DEFAULT 0,\n"
			"UNIQUE(" Img_dir_id ", " Img_file_name ")\n"
			"FOREIGN KEY(" Img_dir_id ") REFERENCES " ImageDir_Table "(" Img_id ")\n"
			")"
		);

		sql::Query(
			"CREATE TABLE " TagILink_Table " (\n"
			TIL_image_id	" INTEGER NOT NULL,\n"
			TIL_tag_id		" INTEGER NOT NULL,\n"
			"PRIMARY KEY(" TIL_image_id ", " TIL_tag_id "),\n"
			"FOREIGN KEY(" TIL_image_id ") REFERENCES " Image_Table "(" Img_id "),\n"
			"FOREIGN KEY(" TIL_tag_id ") REFERENCES " Tag_Table "(" Tag_id ")\n"
			")"
		);

		sql::Query(
			"CREATE TABLE " TagDLink_Table " (\n"
			TDL_tag_id		" INTEGER NOT NULL,\n"
			TDL_dir_id		" INTEGER NOT NULL,\n"
			"PRIMARY KEY(" TDL_tag_id ", " TDL_dir_id "),\n"
			"FOREIGN KEY(" TDL_tag_id ") REFERENCES " Tag_Table "(" Tag_id "),\n"
			"FOREIGN KEY(" TDL_dir_id ") REFERENCES " ImageDir_Table "(" IDir_id ")\n"
			")"
		);

		sql::Query(
			"CREATE TABLE " Tag_Table " (\n"
			Tag_id			" INTEGER PRIMARY KEY,\n"
			Tag_name		" TEXT NOT NULL,\n"
			"UNIQUE (" Tag_name ")\n"
			")"
		);
	}
	std::optional<ImageId> Database::_addImage(const DirId dirId, const QString& path) {
		// 有効な画像だけリストに加える
		try {
			const ImageInfo info(path);
			return info.addEntry(dirId);
		} catch(ImageInfo::CantLoad) {
			return std::nullopt;
		}
	}
	void Database::_validation() const {
		#ifdef QT_DEBUG
		qDebug() << "Validation begin...";
		// 登録されているImageが正当なものかチェック
		{
			auto q = sql::Query(
				"WITH RECURSIVE r(id, path) AS (\n"
				"	SELECT " IDir_id ", " IDir_path " || '/' || " IDir_name " FROM " ImageDir_Table " WHERE " IDir_parent_id " IS NULL\n"
				"	UNION ALL\n"
				"	SELECT img." IDir_id ", r.path || '/' || img." IDir_name " FROM r, " ImageDir_Table " img\n"
				"		WHERE img." IDir_parent_id " = r.id\n"
				")\n"
				"SELECT r.path || '/' || img." Img_file_name ", img." Img_file_name ", " Img_width ", " Img_height ", " Img_area ", " Img_aspect ", " Img_hash ", " Img_modify_date "\n"
				"	FROM " Image_Table " img INNER JOIN r\n"
				"		ON img." Img_dir_id " = r.id\n"
			);
			while(q.next()) {
				struct Column {
					enum {
						FullPath,
						FileName,
						Width,
						Height,
						Area,
						Aspect,
						Hash,
						ModifyDate
					};
				};
				// ファイルの中身を確認(サイズやハッシュ、ファイル時刻)
				const ImageInfo img(q.value(Column::FullPath).toString());
				const auto chk = [&q](const auto& expect, const auto column){
					using typ = std::decay_t<decltype(expect)>;
					if(expect != sql::ConvertQV<typ>(q.value(column)))
						throw 0;
				};
				chk(img.fileName, Column::FileName);
				chk(img.width(), Column::Width);
				chk(img.height(), Column::Height);
				chk(img.area(), Column::Area);
				chk(img.aspect(), Column::Aspect);
				chk(img.hash, Column::Hash);
				chk(img.fileTime, Column::ModifyDate);
			}
		}
		// cand_flagが1は無い
		Q_ASSERT(
			sql::GetRequiredValue<int>(
				sql::Query(
					"SELECT COUNT(*) FROM " Image_Table " WHERE " Img_cand_flag "=1"
				)
			) == 0
		);
		// Dirタグの接合性チェック
		const auto expect = sql::GetRequiredValue<int32_t>(
							sql::Query("SELECT COUNT(*) FROM " TagDLink_Table)),
					actual = sql::GetRequiredValue<int32_t>(
							sql::Query(
								"SELECT COUNT(*) FROM " TagDLink_Table " tdl\n"
								"	INNER JOIN " Tag_Table " tag ON tdl." TDL_tag_id "=tag." Tag_id "\n"
								"	INNER JOIN " ImageDir_Table " idir ON tdl." TDL_dir_id "=idir." Tag_id"\n"
								"WHERE idir." IDir_name "=tag." Tag_name
							));
		Q_ASSERT(expect == actual);
		// 登録されているImageに対してcheckImageを呼ぶと全部NoUpdateが返る
		{
			const auto ids = sql::GetValues<ImageId>(
				sql::Query(
					"SELECT " Img_id " FROM " Image_Table
				)
			);
			for(auto id : ids) {
				Q_ASSERT(const_cast<Database*>(this)->_checkImage(id, false) == CheckResult::NoUpdate);
			}
		}
		// タグが何処にも(TagILink, TagDLink)使われていないのに残っていないかチェック
		{
			auto q = sql::Query(
				"SELECT " Tag_id " FROM " Tag_Table
			);
			while(q.next()) {
				const auto tagId = sql::GetRequiredValue<TagId>(q, 0, false);
				const auto found = sql::GetRequiredValue<int>(
					sql::Query(
						"SELECT\n"
						"	EXISTS(\n"
						"		SELECT id FROM " Image_Table " img\n"
						"			INNER JOIN " TagILink_Table " link\n"
						"			ON img." Img_id "=link." TIL_image_id " AND link." TIL_tag_id "=?\n"
						"		UNION ALL\n"
						"		SELECT id FROM " ImageDir_Table " dir\n"
						"			INNER JOIN " TagDLink_Table " link\n"
						"			ON dir." IDir_id "=link." TDL_dir_id " AND link." TDL_tag_id "=?\n"
						"	)",
						tagId,
						tagId
					)
				);
				Q_ASSERT(found == 1);
			}
		}
		// ImageDirがルート以下すべて登録されているかチェック
		{
			const auto root = getRootDir();
			for(auto rootId : root) {
				// ファイルシステムのディレクトリを走査
				_imageDirValidation(_getFullPath(rootId));
			}
		}
		qDebug() << "Validation end...";
		#endif
	}
	DirIdOpt Database::_dirFromPath(const QString& path) const {
		QFileInfo info(path);
		if(info.exists()) {
			const auto d = sql::GetValues<DirId>(
				sql::Query(
					"SELECT " IDir_id " FROM " ImageDir_Table " WHERE " IDir_name "=?",
					info.fileName()
				)
			);
			for(auto id : d) {
				if(path == _getFullPath(id))
					return id;
			}
		}
		return std::nullopt;
	}
	void Database::_imageDirValidation(const QString& path) const {
		// DBに登録されているか
		const auto dId = _dirFromPath(path);
		Q_ASSERT(dId);
		// 下層のチェック
		size_t nSub_fs = 0;
		_EnumSubDir(
			path,
			[&nSub_fs, this](const QFileInfo& info){
				_imageDirValidation(info.absoluteFilePath());
				++nSub_fs;
			}
		);
		// 子ディレクトリの数を比較
		const auto nSub_db = sql::GetRequiredValue<size_t>(
			sql::Query(
				"SELECT COUNT(" IDir_id ") FROM " ImageDir_Table " WHERE " IDir_parent_id "=?",
				*dId
			)
		);
		Q_ASSERT(nSub_db == nSub_fs);
	}
	bool Database::_checkDatabaseValidness() {
		const QStringList tables = _db.tables();
		if(!tables.contains(Tag_Table))
			return false;
		if(!tables.contains(TagDLink_Table))
			return false;
		if(!tables.contains(TagILink_Table))
			return false;
		if(!tables.contains(ImageDir_Table))
			return false;
		if(!tables.contains(Image_Table))
			return false;
		return true;
	}
	void Database::_EnumThumbnailFile(const CBThumbnail& cb) {
		const auto files = QDir(_ThumbnailLocation(false)).entryInfoList(
			{},
			QDir::Files | QDir::Writable | QDir::NoDotAndDotDot
		);
		for(const QFileInfo& f : files) {
			cb(f);
		}
	}
	void Database::_clearThumbnail() {
		_EnumThumbnailFile([](const QFileInfo& f){
			QFile(f.absoluteFilePath()).remove();
		});
	}
	void Database::_RemoveThumbnailFile(const ImageId id) {
		const QDir dir(_ThumbnailLocation(false));
		QFile f(dir.absoluteFilePath(_ThumbnailFileName(id)));
		if(f.exists())
			f.remove();
	}
	QString Database::_ThumbnailFileName(const ImageId id) {
		// ファイル名 = ImageId.jpg
		return QString("%1.jpg").arg(id);
	}
	void Database::_saveThumbnailAsFile() {
		const QDir thumb_dir(_ThumbnailLocation(false));
		// 既にメモリ上にキャッシュしてある分だけをファイルとして保存
		auto itr = _thumbnail.begin(),
			 itrE = _thumbnail.end();
		while(itr != itrE) {
			const auto id = itr.key();
			const auto path = thumb_dir.filePath(_ThumbnailFileName(id));
			QFileInfo fi(path);
			// 既にファイルがある場合は何もしない
			if(fi.exists()) {}
			else {
				itr.value().save(path);
			}
			++itr;
		}
	}
	QPixmap Database::_LoadThumbnailFromFile(const ImageId id) {
		const QDir dir(_ThumbnailLocation(false));
		QFileInfo f(dir.filePath(_ThumbnailFileName(id)));
		if(f.exists()) {
			return QPixmap(f.filePath());
		}
		return QPixmap();
	}
	namespace {
		QString CreateIfNeeded(const QString& path, const bool create) {
			QDir dir(path);
			const bool exists = dir.exists();
			if(create) {
				if(!exists) {
					if(!dir.mkpath(QStringLiteral(".")))
						throw std::runtime_error("mkpath");
				}
			} else {
				if(!exists) {
					throw std::runtime_error(path.toStdString() + " Location not found");
				}
			}
			return path;
		}
	}
	QString Database::_ThumbnailLocation(const bool create) {
		return CreateIfNeeded(_CacheLocation(create) % "/" % c_thumbnailDir, create);
	}
	QString Database::_CacheLocation(const bool create) {
		return CreateIfNeeded(QStandardPaths::writableLocation(QStandardPaths::CacheLocation), create);
	}
	QSqlDatabase Database::_makeConnection() {
		QSqlDatabase db = QSqlDatabase::addDatabase(c_dbType);
		// featureの確認
		if(!db.driver()->hasFeature(QSqlDriver::BLOB))
			throw std::runtime_error("Feature(BLOB) is not supported");
		if(!db.driver()->hasFeature(QSqlDriver::Transactions))
			throw std::runtime_error("Feature(Transactions) is not supported");

		_remc.del = true;
		return db;
	}
	bool Database::_initDatabase(const bool createFile) {
		bool ret = false;
		// データベースファイルの読み込み（作成）
		{
			QDir dir(_CacheLocation(true));
			if(createFile) {
				dir.remove(c_databaseFile);
				ret = true;
			} else if(!QFileInfo::exists(dir.filePath(c_databaseFile))) {
				ret = true;
			}
			_db.setDatabaseName(dir.absoluteFilePath(c_databaseFile));
		}
		if(!_db.open())
			throw std::runtime_error(_db.lastError().text().toStdString());
		sql::Query("PRAGMA foreign_keys = ON");
		return ret;
	}
	void Database::_updateDatabaseLocal(const DirId dirId, const QString& path) {
		// Imageの検査
		// 既に存在が確認されたファイル名リスト
		QSet<QString> exist;
		auto q = sql::Query(
			"SELECT " Img_id ", " Img_file_name " FROM " Image_Table " WHERE " Img_dir_id "=?",
			dirId
		);
		while(q.next()) {
			const auto imgId = sql::GetRequiredValue<ImageId>(q, 0, false);
			const auto res = _checkImage(imgId, false);
			if(res == CheckResult::Deleted) {
				// サムネイルも消す
				_removeThumbnail(imgId);
			}
			else
				exist.insert(sql::GetRequiredValue<QString>(q, 1, false));
		}
		// ファイルシステムを巡り、新規に追加されたファイルを探す
		ResetSignal sig(this);
		_collectImageInDir(sig, path, dirId, [&exist](const QFileInfo& f){
			// existに登録がない = 新しく追加された物
			return exist.contains(f.fileName()) == 0;
		});

		exist.clear();
		// 下層のチェック
		for(const auto cId : getDirChild(dirId)) {
			const auto cPath = _getFullPath(cId);
			QFileInfo fi(cPath);
			if(fi.exists()) {
				exist.insert(fi.fileName());
				// 下層のディレクトリを探索
				_updateDatabaseLocal(cId, cPath);
			} else {
				// 削除されている
				_removeDirPrivate(cId, false);
			}
		}
		// ファイルシステムを巡り、新規に追加されたサブディレクトリを探す
		_EnumSubDir(path, [this, &sig, &exist, dirId](const QFileInfo& f){
			if(!exist.contains(f.fileName())) {
				_addDir(sig, f.absoluteFilePath(), dirId);
			}
		});
	}
	void Database::_updateDatabase() {
		// DB上のファイル構造とファイルシステムのファイル構造を比較
		const auto root = getRootDir();
		for(auto id : root) {
			const QString path = _getFullPath(id);
			if(QFileInfo::exists(path))
				_updateDatabaseLocal(id, path);
			else
				removeDir(id);
		}
	}
	namespace setting {
		const QString version("version");
	}
	bool Database::_checkAppVersion() const {
		QSettings s;
		s.beginGroup(setting::version);
		if(const auto confVer = Version::FromSettings(s)) {
			const auto thisVer = Version::ThisVersion();
			if(!(thisVer == *confVer))
				return false;
		} else
			return false;
		return true;
	}
	void Database::_init(bool clear) {
		Q_ASSERT(!_db.isValid());
		_db = _makeConnection();
		// 設定ファイルのバージョンを比較し、互換性が無かったら破棄
		if(!_checkAppVersion())
			clear = true;

		if(_initDatabase(clear))
			clear = true;
		else if(!_checkDatabaseValidness())
			clear = true;

		// サムネディレクトリの用意(mkdir)
		_ThumbnailLocation(true);

		if(clear) {
			// データの作り直し
			_createTable();
			// サムネ全消し
			_clearThumbnail();
		} else {
			// 半端なcand_flagをリセット
			resetSelectionFlag();
			// データの更新
			_updateDatabase();
		}
		_validation();
		_ct = new ColumnTarget(this);
	}
	Database::Database(QObject* parent):
		DatabaseSignal(parent)
	{
		_init(false);
	}
	Database::Database(_tagInit, QObject* parent):
		DatabaseSignal(parent)
	{
		_init(true);
	}
	Database::~Database() {
		_saveThumbnailAsFile();
		// アプリケーションのバージョンを書き込む
		_writeAppVersion();
	}
	void Database::_writeAppVersion() const {
		QSettings s;
		s.beginGroup(setting::version);
		Version::ThisVersion().writeSettings(s);
	}
	const ColumnTarget* Database::getImageColumnTarget() const {
		return _ct;
	}
	QStringList Database::_ImageFilter() {
		static QStringList ret{"*.png", "*.jpg", "*.bmp", "*.gif"};
		return ret;
	}
	DirIdV Database::getRootDir() const {
		return sql::GetValues<DirId>(
			sql::Query(
				"SELECT " IDir_id " FROM " ImageDir_Table " WHERE " IDir_parent_id " IS NULL\n"
				"	ORDER BY " IDir_id
			)
		);
	}
	QString Database::_getFullPath(const DirId id) const {
		return sql::GetRequiredValue<QString>(
			sql::Query(
				// [累積パス], [親Id], [現在のId]
				"WITH RECURSIVE r(accum, next_id, id) AS (\n"
				"	SELECT " IDir_name ", " IDir_parent_id ", " IDir_id " FROM " ImageDir_Table " WHERE " IDir_id "=?\n"
				"	UNION ALL\n"
				"	SELECT img." IDir_name " || '/' || r.accum, img." IDir_parent_id ", img." IDir_id "\n"
				"	FROM " ImageDir_Table " img, r\n"
				"	WHERE img." IDir_id " = r.next_id\n"
				")\n"
				"SELECT img." IDir_path " || '/' || r.accum\n"
				"	FROM r\n"
				"		INNER JOIN " ImageDir_Table " img\n"
				"		ON r.id = img." IDir_id "\n"
				"	WHERE next_id IS NULL;\n",
				id
			)
		);
	}
	DirId Database::_getRootDir(const DirId id) const {
		auto q = sql::Query(
			"WITH RECURSIVE r(id, parent_id) AS (\n"
			"	SELECT " IDir_id ", " IDir_parent_id " FROM " ImageDir_Table " WHERE " IDir_id "=?\n"
			"	UNION ALL\n"
			"	SELECT img." IDir_id ", img." IDir_parent_id " FROM " ImageDir_Table " img, r\n"
			"		WHERE img." IDir_id " = r.parent_id\n"
			")\n"
			"SELECT id FROM r WHERE parent_id IS NULL",
			id
		);
		const auto r_id = sql::GetValue<DirId>(q);
		Q_ASSERT(r_id);
		return *r_id;
	}
	void Database::_addDirPrivate(const QString& path, const DirIdOpt parent) {
		QDir dir(path);
		if(!dir.exists())
			throw std::runtime_error("invalid path");
		const QString absPath(dir.absolutePath());
		// 既にディレクトリが登録されていたら何もしない
		{
			auto q = sql::Query(
				"SELECT " IDir_id " FROM " ImageDir_Table " WHERE " IDir_parent_id " IS NULL AND " IDir_path "||'/'||" IDir_name "=?",
				absPath
			);
			if(sql::GetValue<DirId>(q))
				return;
		}
		// 子ディレクトリが既に登録されていないか
		auto q = sql::Query(
			"SELECT " IDir_id " FROM " ImageDir_Table "\n"
			"	WHERE " IDir_parent_id " IS NULL AND " IDir_path "||'/'||" IDir_name " LIKE ?",
			absPath + "/%"
		);
		while(q.next()) {
			// 一旦削除
			removeDir(sql::GetRequiredValue<DirId>(q, 0, false));
		}

		ResetSignal sig(this);
		// 親(のほうの)Dirが既に登録されていたら何もせず終了
		if(!parent) {
			QDir d(dir);
			for(;;) {
				auto d2 = d;
				if(!d2.cdUp())
					break;
				if(sql::FindEntryId(
					ImageDir_Table,
					IDir_id,
					IDir_name,			d.dirName(),
					IDir_path,			d2.absolutePath()
				))
					return;
				d = d2;
			}
		}
		_addDir(sig, path, parent);

		_validation();
	}
	void Database::addDir(const QString& path) {
		_addDirPrivate(path, std::nullopt);
	}
	void Database::_removeDirSingle(ResetSignal& sig, const DirId id) {
		// ASSERT: 末端ディレクトリしか対応しないので、そのチェック
		Q_ASSERT(
			sql::GetRequiredValue<int>(
				sql::Query(
					"SELECT COUNT(" IDir_parent_id ") FROM " ImageDir_Table " WHERE " IDir_parent_id "=?",
					id
				)
			) == 0
		);
		// TagDLinkエントリの削除
		const auto tagId = sql::GetRequiredValue<TagId>(
			sql::Query(
				"SELECT " TDL_tag_id " FROM " TagDLink_Table " WHERE " TDL_dir_id "=?",
				id
			)
		);
		{
			const auto q = sql::Query(
				"DELETE FROM " TagDLink_Table " WHERE " TDL_dir_id "=?",
				id
			);
			Q_ASSERT(q.numRowsAffected() == 1);
		}

		// Dirに含まれるImageに付けられたTagILinkエントリの削除
		sql::Query(
			"DELETE FROM " TagILink_Table "\n"
			"	WHERE EXISTS(\n"
			"		SELECT 1 FROM " Image_Table " img\n"
			"		WHERE img." Img_dir_id "=? AND " TIL_image_id "=img." Img_id "\n"
			"	)",
			id
		);

		// 他に同名のDirが無ければDirタグを削除
		{
			const QString name = sql::GetRequiredValue<QString>(
				sql::Query(
					"SELECT " IDir_name " FROM " ImageDir_Table "\n"
					"	WHERE " IDir_id "=?",
					id
				)
			);
			if(sql::GetRequiredValue<int32_t>(
				sql::Query(
					"SELECT COUNT(*) FROM " ImageDir_Table "\n"
					"	WHERE " IDir_name "=?",
					name
				)
			) == 1) {
				// (Tagエントリの削除)
				const auto q = sql::Query(
					"DELETE FROM " Tag_Table " WHERE " Tag_id "=?",
					tagId
				);
				Q_ASSERT(q.numRowsAffected() == 1);

				// モデルに通知
				sig.tagChange();
			}
		}
		_removeThumbnailInDir(id);

		// Dirに含まれるImageの削除
		sql::Query(
			"DELETE FROM " Image_Table " WHERE " Img_dir_id "=?",
			id
		);
		sig.imageChange();

		// ImageDirエントリの削除
		{
			const auto q = sql::Query(
				"DELETE FROM " ImageDir_Table " WHERE " IDir_id "=?",
				id
			);
			Q_ASSERT(q.numRowsAffected() == 1);
		}
		sig.dirChange();
	}
	void Database::_removeThumbnailInDir(const DirId id) {
		const auto ids =
			sql::GetValues<ImageId>(
				sql::Query(
					"SELECT " Img_id " FROM " Image_Table " WHERE " Img_dir_id "=?",
					id
				)
			);
		for(const auto img_id : ids) {
			_removeThumbnail(img_id);
		}
	}
	void Database::_removeThumbnail(const ImageId id) {
		// メモリ上のキャッシュを削除
		_thumbnail.remove(id);
		// ファイルのキャッシュを削除
		_RemoveThumbnailFile(id);
	}
	void Database::_removeDirPrivate(DirId id, const bool upToRoot) {
		if(upToRoot) {
			// 一旦データベースが保持してるルートまで遡る
			id = _getRootDir(id);
		}
		// 子ノードの列挙 & パスで降順にソート
		auto q = sql::Query(
			"WITH RECURSIVE r(id, layer) AS (\n"
			"	SELECT " IDir_id ", 0 FROM " ImageDir_Table " WHERE " IDir_id "=?\n"
			"	UNION ALL\n"
			"	SELECT img." IDir_id ", r.layer+1 FROM r, " ImageDir_Table " img\n"
			"		WHERE r.id = img." IDir_parent_id "\n"
			")\n"
			"SELECT id FROM r ORDER BY layer DESC",
			id
		);

		// 葉ノード(長いパス)から順に削除していく
		const DirIdV idv = sql::GetValues<DirId>(q);
		Q_ASSERT(!idv.empty());
		ResetSignal sig(this);
		sql::Transaction(QSqlDatabase::database(),
			[idv, &sig, this](){
				for(auto&& id : idv) {
					_removeDirSingle(sig, id);
				}
			}
		);
	}
	void Database::removeDir(const DirId id) {
		_removeDirPrivate(id, true);
		_validation();
	}

	void Database::_addDir(ResetSignal& sig, const QString& path, const std::optional<DirId> parentId) {
		const QFileInfo info(path);
		if(!info.exists())
			throw std::invalid_argument("invalid path");

		QVariant var_FileName(info.fileName()),
				var_Path,
				var_ParentId;
		if(parentId) {
			var_Path = QVariant(QVariant::String);
			var_ParentId = *parentId;
		} else {
			var_Path = info.path();
			var_ParentId = QVariant(QVariant::Int);
		}

		sig.dirChange();
		// ディレクトリエントリ登録
		const auto dirId = static_cast<DirId>(
			sql::InsertInto_GetId(
				ImageDir_Table,
				IDir_name,			var_FileName,
				IDir_path,			var_Path,
				IDir_parent_id, 	var_ParentId
			)
		);
		// タグの登録
		auto [tagId, tagAdded] = sql::InsertIntoIfNotExist_GetId(
			Tag_Table,
			Tag_name,			info.fileName()
		);
		if(tagAdded) {
			// 本当は追加前に呼ぶ
			sig.tagChange();
		}
		// 関連付け
		sql::InsertInto(
			TagDLink_Table,
			TDL_tag_id,			tagId,
			TDL_dir_id,			dirId
		);
		// 画像の列挙 & 読み込み
		_collectImageInDir(sig, path, dirId, [](auto&&){ return true; });

		// 下層のディレクトリを走査
		_EnumSubDir(path, [this, &sig, dirId](const QFileInfo& info){
			_addDir(sig, info.absoluteFilePath(), dirId);
		});
	}
	void Database::_EnumSubDir(const QString& path, const CBEnumSubDir& cb) {
		const auto dirs = QDir(path).entryInfoList(
			{},
			QDir::Dirs|QDir::Readable|QDir::NoDotAndDotDot
		);
		for(const QFileInfo& d : dirs) {
			if(d.fileName() == "thumbnails")
				continue;
			cb(d);
		}
	}
	DirIdV Database::_enumAncestor(const DirId id) const {
		return sql::GetValues<DirId>(
			sql::Query(
				"WITH RECURSIVE r(id, parent_id) AS (\n"
				"	SELECT " IDir_id ", " IDir_parent_id " FROM " ImageDir_Table " WHERE " IDir_id "=?\n"
				"	UNION ALL\n"
				"	SELECT img." IDir_id ", img." IDir_parent_id " FROM " ImageDir_Table " img, r\n"
				"		WHERE img." IDir_id " = r.parent_id\n"
				")\n"
				"SELECT id FROM r",
				id
			)
		);
	}
	void Database::_EnumImage(const QString& path, const CBEnumImageDir& cb) {
		const QDir dir(path);
		const auto files = dir.entryInfoList(
			_ImageFilter(),
			QDir::Files|QDir::Readable
		);
		for(const QFileInfo& f : files) {
			cb(f);
		}
	}
	void Database::_collectImageInDir(
			ResetSignal& sig,
			const QString& path, const DirId dirId,
			const CBCollectImage& cb
	) {
		// pathは有効
		Q_ASSERT(QFileInfo::exists(path));
		// DirIdは登録済み
		Q_ASSERT(_getFullPath(dirId) == path);

		// DirIdから関連タグを再帰クエリで列挙
		const TagIdV tagv = [this, dirId](){
			TagIdV t;
			const auto dirIds = _enumAncestor(dirId);
			for(const auto di : dirIds) {
				t.emplace_back(
					sql::FindEntryIdNum(
						TagDLink_Table,
						TDL_tag_id,
						TDL_dir_id, di
					)
				);
			}
			return t;
		}();

		QVariantList vl_id,
					vl_tagId;
		_EnumImage(path, [&](const QFileInfo& f){
			if(cb(f)) {
				if(const auto imgId = _addImage(dirId, f.absoluteFilePath())) {
					// Dirタグを関連付け
					for(auto tagId : tagv) {
						vl_id.append(*imgId);
						vl_tagId.append(tagId);
					}
				}
			}
		});
		if(!vl_id.empty())
			sig.imageChange();

		QSqlQuery q;
		q.prepare(
			"INSERT INTO " TagILink_Table "(" TIL_image_id ", " TIL_tag_id ")\n"
			"VALUES (?, ?)"
		);
		q.addBindValue(vl_id);
		q.addBindValue(vl_tagId);
		sql::Batch(q);
	}
	QString Database::getFullPath(const ImageId id) const {
		const ImageInfo info = getImageInfo(id);
		return _getFullPath(info.loadedDirId) % '/' % info.fileName;
	}
	QPixmap Database::getThumbnail(const ImageId id) const {
		// メモリ上のキャッシュ
		auto itr = _thumbnail.find(id);
		if(itr != _thumbnail.end()) {
			return itr.value();
		}
		// ファイルキャッシュ
		QPixmap p = _LoadThumbnailFromFile(id);
		if(p.isNull()) {
			// サムネイル生成
			const QString fullPath = getFullPath(id);
			QImageReader reader(fullPath);
			reader.setScaledSize(AspectKeepScale({32,32}, reader.size()));
			const QImage img = reader.read();
			Q_ASSERT(!img.isNull());
			p = QPixmap::fromImage(img);
		}
		_thumbnail.insert(id, p);
		return p;
	}
	DirInfo Database::getDirInfo(const DirId id) const {
		auto q = sql::Query(
			"SELECT " IDir_path ", " IDir_name " FROM " ImageDir_Table " WHERE " IDir_id "=?",
			id
		);
		const bool b = q.next();
		Q_ASSERT(b);
		return DirInfo {
			.path = q.value(0).toString(),
			.name = sql::GetRequiredValue<QString>(q, 1, false)
		};
	}
	DirIdV Database::getDirChild(const DirIdOpt id) const {
		if(id) {
			return sql::GetValues<DirId>(
				sql::Query(
					"SELECT " IDir_id " FROM " ImageDir_Table " WHERE " IDir_parent_id "=?\n"
					"	ORDER BY " IDir_id,
					*id
				)
			);
		}
		return getRootDir();
	}
	DirIdOpt Database::getDirParent(const DirId id) const {
		return sql::GetValue<DirId>(
			sql::Query(
				"SELECT " IDir_parent_id " FROM " ImageDir_Table " WHERE " IDir_id "=?",
				id
			)
		);
	}
	ImageDirModel* Database::makeDirModel(QObject* parent) {
		auto* ret = new ImageDirModel(this, parent);
		connect(this, &Database::beginResetDir,
				ret, &ImageDirModel::beginReset);
		connect(this, &Database::endResetDir,
				ret, &ImageDirModel::endReset);
		return ret;
	}
	QSqlTableModel* Database::makeTagModel(QObject* parent) const {
		auto* m = new QSqlTableModel(parent);
		m->setTable(Tag_Table);
		connect(this, &Database::beginResetTag,
			m, [m](){
			m->setProperty("_filter", m->filter());
			m->setFilter("False");
			m->select();
		});
		connect(this, &Database::endResetTag,
			m, [m](){
			m->setFilter(m->property("_filter").toString());
			m->select();
		});
		m->select();
		return m;
	}
	ImageModel_p Database::makeImageModel(QObject* parent) const {
		auto* m = new ImageQueryModel(this, this, parent);
		m->setTable(Image_Table);
		connect(this, &Database::beginResetImage,
				m, &ImageQueryModel::beginReset);
		connect(this, &Database::endResetImage,
				m, &ImageQueryModel::endReset);
		m->select();

		auto* mp = new CheckBoxProxy(m->getColumnTarget(), parent);
		mp->setSourceModel(m);
		auto* del = new CheckBoxDelegate(m->getColumnTarget(), parent);
		return std::make_tuple(mp, m, del);
	}
	size_t Database::numImagesInDir(const DirId id) const {
		return sql::GetRequiredValue<size_t>(
			sql::Query(
				"SELECT COUNT(" Img_id ") FROM " Image_Table " WHERE " Img_dir_id "=?",
				id
			)
		);
	}
	size_t Database::numTotalImagesInDir(const DirId id) const {
		size_t sum = numImagesInDir(id);
		for(const auto c : getDirChild(id)) {
			sum += numTotalImagesInDir(c);
		}
		return sum;
	}
	TagIdOpt Database::getTagId(const QString& name) const {
		if(const auto ret = sql::GetValue<TagId>(
			sql::Query("SELECT " Tag_id " FROM " Tag_Table " WHERE LOWER(" Tag_name ")=LOWER(?)", name)
		))
			return *ret;
		return std::nullopt;
	}
	QString Database::getTagName(const TagId id) const {
		if(const auto ret = sql::GetValue<QString>(
			sql::Query("SELECT " Tag_name " FROM " Tag_Table " WHERE " Tag_id "=?", id))
		)
			return *ret;
		return QString();
	}
	std::pair<size_t,size_t> Database::countImageByTag(const TagIdV& tag) const {
		const auto sq = tagMatchQuery({Img_id, Img_cand_flag}, tag);
		return {
			sql::GetRequiredValue<size_t>(
				sql::Query(
					QString("SELECT COUNT(*) FROM (%1)")
					.arg(sq)
				)
			),
			sql::GetRequiredValue<size_t>(
				sql::Query(
					QString("SELECT COUNT(*) FROM (%1) WHERE " Img_cand_flag "=2")
					.arg(sq)
				)
			)
		};
	}
	ImageIdV Database::findImageByTag(const TagIdV& tag) const {
		return sql::GetValues<ImageId>(
			sql::Query(
				tagMatchQuery({"id"}, tag)
			)
		);
	}
	namespace {
		QString MakeWithClause(const QStringList& lst) {
			return "WITH RECURSIVE " + lst.join(",\n");
		}
	}
	ImageCandV Database::enumImageByAspect(
		const TagIdV& tag,
		size_t nBucket,
		const size_t maxBucket
	) {
		// ---------- サブクエリ: cand_img ----------
		const auto cand_img = QString(
				"cand_img(image_id, width, height, aspect, cand_flag) AS (SELECT * FROM (%1) %2)\n"
			)
			.arg(tagMatchQuery({Img_id, Img_width, Img_height, Img_aspect, Img_cand_flag}, tag))
			.arg("WHERE " Img_cand_flag "=0");

		// aspectの最大最小値を取得
		auto minmax = sql::Query(
			MakeWithClause({cand_img})
			% "SELECT MIN(aspect), MAX(aspect) FROM cand_img"
		);
		const bool chk = minmax.next();
		Q_ASSERT(chk);
		if(minmax.value(0).isNull()) {
			// 一件も画像が無い
			return {};
		}
		const auto min = sql::GetRequiredValue<int>(minmax, 0, false),
				max = sql::GetRequiredValue<int>(minmax, 1, false)+1;
		Q_ASSERT(min >= 0 && min <= max);
		nBucket = static_cast<size_t>(std::min(int(nBucket), max-min));

		// aspect範囲の分だけbucketを生成
		QString collector("collect_img(image_id, range_num, sort_num) AS (\n");
		const QString base = QString("SELECT * FROM (SELECT image_id, %1, %1 + ROW_NUMBER() OVER(ORDER BY aspect) * 10000 FROM cand_img\n") %
									"WHERE aspect BETWEEN %2 AND %3 LIMIT %4)";
		int lc = min,
			rc = min;
		int cur = 0;
		const auto add = [&collector, &base, maxBucket, min, &lc, &rc, &cur](){
			// union allで結合
			if(lc > min)
				collector += "\nUNION ALL\n";
			// add clause(lc, rc)
			collector += base
						.arg(cur++)
						.arg(lc)
						.arg(rc)
						.arg(maxBucket);
		};
		int err = (max-min)/2;
		while(rc < max) {
			err -= (nBucket-1);
			if(err <= 0) {
				err += max-min;
				add();
				lc = rc+1;
			}
			++rc;
		}
		if(lc != rc) {
			add();
		}
		collector += ")";

		const QString qs =
			MakeWithClause({cand_img, collector}) % "\n" %
			QString(
				"SELECT c.image_id, ci.width, ci.height\n"
				"	FROM collect_img c\n"
				"	INNER JOIN cand_img ci\n"
				"		ON c.image_id = ci.image_id\n"
				"	ORDER BY c.sort_num"
			);
		auto q = sql::Query(qs);
		ImageCandV img;
		while(q.next()) {
			img.emplace_back(
				ImageCand {
					.id = sql::GetRequiredValue<ImageId>(q, 0, false),
					.size = QSize {
						sql::GetRequiredValue<int>(q, 1, false),
						sql::GetRequiredValue<int>(q, 2, false)
					}
				}
			);
		}
		return img;
	}
	void Database::setViewFlag(const ImageIdV& img, const int num) {
		QVariantList vl_id;
		for(auto id : img)
			vl_id.append(id);

		emit beginResetImage();
		{
			QSqlQuery q;
			q.prepare(
				QString("UPDATE " Image_Table " SET " Img_cand_flag "=%1 WHERE " Img_id "=?")
				.arg(num)
			);
			q.addBindValue(vl_id);
			sql::Batch(q);
		}
		emit endResetImage();
	}
	void Database::resetSelectionFlag() {
		emit beginResetImage();
		sql::Query("UPDATE " Image_Table " SET " Img_cand_flag "=0 WHERE " Img_cand_flag "=1");
		emit endResetImage();
	}
	QString Database::tagMatchQuery(QStringList getcol, const TagIdV& tag) const {
		if(tag.empty())
			return QString(
				"SELECT %1 FROM " Image_Table " img"
			).arg(getcol.join(','));

		QStringList tags;
		for(auto id : tag)
			tags.append(QString("%1").arg(id));

		for(auto& c : getcol) {
			c = QStringLiteral("img.") % c;
		}

		return QString(
			"SELECT %3 FROM " Image_Table " img\n"
			"	INNER JOIN " TagILink_Table " link\n"
			"	ON img." Img_id "=link." TIL_image_id "\n"
			"	WHERE (link." TIL_tag_id " IN (%1))\n"
			"	GROUP BY img." Img_id "\n"
			"	HAVING COUNT(img." Img_id ")=%2"
		)
		.arg(tags.join(','))
		.arg(tags.size())
		.arg(getcol.join(','));
	}
	ImageInfo Database::getImageInfo(const ImageId id) const {
		auto q = sql::Query(
			"SELECT " Img_file_name ", " Img_width ", " Img_height ", " Img_hash ", " Img_modify_date ", " Img_dir_id ", " Img_cand_flag "\n"
			"	FROM " Image_Table " WHERE " Img_id "=?",
			id
		);
		if(!q.next()) {
			throw std::runtime_error("invalid image id");
		}
		ImageInfo info;
		info.fileName = sql::GetRequiredValue<QString>(q, 0, false);
		info.size = {
			sql::GetRequiredValue<int>(q, 1, false),
			sql::GetRequiredValue<int>(q, 2, false)
		};
		info.hash = sql::GetRequiredValue<QByteArray>(q, 3, false);
		info.fileTime = sql::GetRequiredValue<qint64>(q, 4, false);
		info.loadedDirId = sql::GetRequiredValue<DirId>(q, 5, false);
		info.candFlag = sql::GetRequiredValue<int>(q, 6, false);
		return info;
	}
	TagIdV Database::enumTagForwardMatch(const QString& str) const {
		return sql::GetValues<TagId>(
			sql::Query(
				"SELECT " Tag_id " FROM " Tag_Table " WHERE " Tag_name " LIKE ? ORDER BY " Tag_name,
				str + '%'
			)
		);
	}
	void Database::resetViewFlag() {
		// 表示フラグを全部リセット
		if(sql::GetRequiredValue<size_t>(
			sql::Query("SELECT COUNT(" Img_id ") FROM " Image_Table " WHERE " Img_cand_flag ">0")
		) > 0)
		{
			emit beginResetImage();
			const auto q = sql::Query(
				"UPDATE " Image_Table " SET " Img_cand_flag "=0 WHERE " Img_cand_flag ">0"
			);
			Q_ASSERT(q.numRowsAffected() > 0);
			emit endResetImage();
		}
	}
	TagIdV Database::excludeRemovedTag(const TagIdV& tag) const {
		TagIdV ret;
		for(const auto id : tag) {
			if(sql::GetRequiredValue<bool>(
				sql::Query("SELECT COUNT(" Tag_id ") FROM " Tag_Table " WHERE " Tag_id "=?", id)
			))
				ret.emplace_back(id);
		}
		return ret;
	}
}
