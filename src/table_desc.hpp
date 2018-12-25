#pragma once

/*
	-- 画像ファイルがあるディレクトリ
	CREATE TABLE ImageDir (
		id			INTEGER PRIMARY KEY,			-- 通し番号
		name		TEXT NOT NULL,					-- ディレクトリ名
		path		TEXT,							-- parent_idを持ってなければルートからの絶対パス
		parent_id	INTEGER,						-- 親ディレクトリがあればそのId
		CHECK((path IS NULL) <> (parent_id IS NULL)),
		UNIQUE(parent_id, name),
		FOREIGN KEY(parent_id) REFERENCES ImageDir(id)
	);
*/
#define ImageDir_Table	"ImageDir"
#define IDir_id			"id"
#define IDir_name		"name"
#define IDir_path		"path"
#define IDir_parent_id	"parent_id"

/*
	-- 画像詳細
	CREATE TABLE Image (
		id				INTEGER PRIMARY KEY,		-- 通し番号
		file_name		TEXT NOT NULL,				-- 画像ファイル名
		dir_id			INTEGER NOT NULL,			-- 画像ファイルがあるディレクトリId
		width			INTEGER NOT NULL,			-- 画像のサイズ(幅)
		height			INTEGER NOT NULL,			-- 画像のサイズ(高さ)
		area			INTEGER NOT NULL,			-- 面積
		aspect			INTEGER NOT NULL,			-- アスペクト比
		hash			BLOB NOT NULL,				-- 画像のSHA1ハッシュ
		modify_date		INTEGER NOT NULL,			-- 詳細をチェックした時のファイル時刻(MSec from Epoch)
		cand_flag		INTEGER DEFAULT 0,			-- 候補フラグ
		UNIQUE(dir_id, file_name)
		FOREIGN KEY(dir_id) REFERENCES ImageDir(id)
	);
*/
#define Image_Table		"Image"
#define Img_id			"id"
#define Img_file_name	"file_name"
#define Img_dir_id		"dir_id"
#define Img_width		"width"
#define Img_height		"height"
#define Img_area		"area"
#define Img_aspect		"aspect"
#define Img_hash		"hash"
#define Img_modify_date	"modify_date"
#define Img_cand_flag	"cand_flag"

struct Image_Column {
	enum {
		Id,
		FileName,
		DirId,
		Width,
		Height,
		Area,
		Aspect,
		Hash,
		ModifyDate,
		CandFlag,
		_Num
	};
};
const char *const Image_CBFlag[] = {};

/*
	-- Imageに関連付けられているタグのリスト
	CREATE TABLE TagILink (
		image_id	INTEGER NOT NULL,
		tag_id		INTEGER NOT NULL,
		PRIMARY KEY(image_id, tag_id),
		FOREIGN KEY(image_id) REFERENCES Image(id),
		FOREIGN KEY(tag_id) REFERENCES Tag(id)
	);
*/
#define TagILink_Table	"TagILink"
#define TIL_image_id	"image_id"
#define TIL_tag_id		"tag_id"

/*
	-- Dir由来のタグ関連付け
	CREATE TABLE TagDLink (
		tag_id		INTEGER NOT NULL,
		dir_id		INTEGER NOT NULL,
		PRIMARY KEY(tag_id, dir_id),
		FOREIGN KEY(tag_id) REFERENCES Tag(id),
		FOREIGN KEY(dir_id) REFERENCES ImageDir(id)
	);
*/
#define TagDLink_Table	"TagDLink"
#define TDL_tag_id		"tag_id"
#define TDL_dir_id		"dir_id"

/*
	-- タグ詳細
	CREATE TABLE Tag (
		id INTEGER PRIMARY KEY,
		name TEXT NOT NULL,							-- タグ名
		UNIQUE (name)
	);
*/
#define Tag_Table		"Tag"
#define Tag_id			"id"
#define Tag_name		"name"
