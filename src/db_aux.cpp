#include "database.hpp"

namespace dg {
	// ---------------- Database::ChangeFlag ----------------
	Database::ChangeFlag::ChangeFlag():
		_changed(false)
	{}
	bool Database::ChangeFlag::change() {
		if(!_changed) {
			_changed = true;
			return true;
		}
		return false;
	}
	bool Database::ChangeFlag::end() {
		return _changed;
	}

	// ---------------- Database::ResetSignal ----------------
	Database::ResetSignal::ResetSignal(Database* db):
		_db(db)
	{}
	Database::ResetSignal::~ResetSignal() {
		if(std::uncaught_exceptions() == 0) {
			if(_dir.end())
				emit _db->endResetDir();
			if(_tag.end())
				emit _db->endResetTag();
			if(_image.end())
				emit _db->endResetImage();
		}
	}
	void Database::ResetSignal::dirChange() {
		if(_dir.change())
			emit _db->beginResetDir();
	}
	void Database::ResetSignal::imageChange() {
		if(_image.change())
			emit _db->beginResetImage();
	}
	void Database::ResetSignal::tagChange() {
		if(_tag.change())
			emit _db->beginResetTag();
	}
}
