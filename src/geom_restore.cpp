#include "geom_restore.hpp"
#include <QSettings>
#include <QShowEvent>

namespace dg {
	namespace {
		const QString c_key("geometry");
	}
	GeomRestore::GeomRestore(const QString& key, QWidget* parent):
		Obstacle(parent),
		_key(key)
	{}
	void GeomRestore::showEvent(QShowEvent* e) {
		if(!e->spontaneous() && _first) {
			_first = false;

			QSettings s;
			s.beginGroup(_key);
			restoreGeometry(s.value(c_key).toByteArray());
		}
		Obstacle::showEvent(e);
	}
	void GeomRestore::closeEvent(QCloseEvent* e) {
		QSettings s;
		s.beginGroup(_key);
		s.setValue(c_key, saveGeometry());
		emit onClose();
		QWidget::closeEvent(e);
	}
}
