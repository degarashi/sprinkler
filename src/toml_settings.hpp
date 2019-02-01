#pragma once
#include "spine/src/singleton.hpp"
#include "toml11/toml.hpp"
#include <QObject>
#include <QSet>
#include <shared_mutex>

class QThread;
class QFileSystemWatcher;
class QTimer;
namespace dg {
	class TomlLoader :
		public QObject
	{
		Q_OBJECT
		private:
		public:
			using QObject::QObject;
		public slots:
			void loadToml(const QString& path);
		signals:
			void loadFailed(const QString& path, const QString& errMsg);
			void loadSucceeded(const QString& path, toml::table table);
	};
	// Tomlで色々な設定値を読み込むクラス(シングルトン)
	// 設定値の変更を監視し、別スレッドで再読み込み
	// 現時点では固定ファイル1つのみに対応
	#define tomlset (::dg::TomlSettings::ref())
	class TomlSettings :
		public QObject,
		public spi::Singleton<TomlSettings>
	{
		Q_OBJECT
		private:
			QThread					*_thread;
			TomlLoader				*_tomlLoader;
			QFileSystemWatcher		*_watcher;
			QTimer					*_checkTimer;

			mutable std::shared_mutex	_mutex;
			toml::table					_table;
			bool						_first;

			QSet<QString>			_checkFile;

			void _loadToml(const QString& path, bool wait);
			toml::value _value(const toml::value& val, const QString& ent) const;

		public:
			TomlSettings(const QString& path);
			~TomlSettings();
			// ドットを挟んだエントリ名に対応
			toml::value value(const QString& ent) const;
			template <class T>
			T toValue(const QString& ent) const {
				return toml::get<T>(value(ent));
			}
	};
	#define Define_TomlSet(table, name, ent, type) \
		auto name() { \
			return tomlset.toValue<type>( \
				QStringLiteral(table "." ent) \
			); \
		}
}
