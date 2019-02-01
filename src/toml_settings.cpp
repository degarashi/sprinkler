#include "toml_settings.hpp"
#include <QThread>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>

namespace dg {
	void TomlLoader::loadToml(const QString& path) {
		try {
			auto table = toml::parse(path.toStdString());
			emit loadSucceeded(path, std::move(table));
		} catch(const std::exception& e) {
			emit loadFailed(path, e.what());
		}
	}

	namespace {
		constexpr const int CheckInterval = 100;
	}
	TomlSettings::TomlSettings(const QString& path):
		_thread(new QThread(this)),
		_tomlLoader(new TomlLoader),
		_watcher(new QFileSystemWatcher(this)),
		_checkTimer(new QTimer(this)),
		_first(true)
	{
		qRegisterMetaType<toml::table>("toml::table");
		_thread->start();
		_checkTimer->setInterval(CheckInterval);
		connect(_checkTimer, &QTimer::timeout,
				this, [this](){
					Q_ASSERT(!_checkFile.empty());
					QSet<QString> notyet;
					for(auto&& cf : _checkFile) {
						if(QFileInfo::exists(cf)) {
							// 再度監視登録
							_watcher->addPath(cf);
							_loadToml(cf, false);
						} else
							notyet.insert(cf);
					}
					_checkFile.clear();
					if(notyet.empty())
						_checkTimer->stop();
					else
						_checkFile = notyet;
				});
		if(! _watcher->addPath(path))
			throw std::runtime_error("Can't open Toml-settings file");
		connect(_tomlLoader, &TomlLoader::loadSucceeded,
				this, [this](const QString&, toml::table table){
					std::lock_guard lk(_mutex);
					_table = std::move(table);
					_first = false;
					qDebug() << "TomlLoader: loadSucceeded";
				});
		connect(_tomlLoader, &TomlLoader::loadFailed,
				this, [this](const QString&, const QString& errMsg){
					const auto stdstr = errMsg.toStdString();
					std::lock_guard lk(_mutex);
					if(_first)
						throw std::runtime_error(stdstr);
					_first = false;
					qDebug() << "TomlLoader: loadFailed:\n" << stdstr.c_str();
				});
		connect(_watcher, &QFileSystemWatcher::fileChanged,
					this, [this](const QString& path){
						if(!QFileInfo::exists(path)) {
							// 削除された場合は一定間隔で再度読み込みを試みる
							_checkFile.insert(path);
							_checkTimer->start();
						} else {
							_watcher->addPath(path);
							// 新しく値を読む(Loaderに任せる)
							_loadToml(path, false);
						}
					});
		_tomlLoader->moveToThread(_thread);

		// 最初のロード
		_loadToml(path, true);
	}
	void TomlSettings::_loadToml(const QString& path, const bool wait) {
		QMetaObject::invokeMethod(
			_tomlLoader,
			"loadToml",
			Q_ARG(QString, path)
		);
		if(wait) {
			for(;;) {
				constexpr const unsigned long IntervalMS = 50;
				QThread::usleep(IntervalMS * 1000);
				QCoreApplication::processEvents();
				if(!_first)
					break;
			}
		}
	}
	toml::value TomlSettings::_value(const toml::value& val, const QString& ent) const {
		return toml::find<toml::value>(val, ent.toStdString());
	}
	toml::value TomlSettings::value(const QString& ent) const {
		std::shared_lock lk(_mutex);
		toml::value ret = _table;
		const QStringList lst = ent.split(".");
		for(auto&& e : lst) {
			ret = _value(ret, e);
		}
		return ret;
	}
	TomlSettings::~TomlSettings() {
		// Loaderスレッド終了 & 待機
		_thread->quit();
		_thread->wait();
	}
}
