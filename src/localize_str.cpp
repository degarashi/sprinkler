#include "localize_str.hpp"

namespace dg::lcl {
	namespace Watcher_Linux {
		QString Add() {
			return QCoreApplication::translate("Watcher_Linux", "Add");
		}
		QString UnableConnectDisplay() {
			return QCoreApplication::translate("Watcher_Linux", "unable to connect to display");
		}
		QString NoName() {
			return QCoreApplication::translate("Watcher_Linux", "(noname)");
		}
		QString ErrorInSelect() {
			return QCoreApplication::translate("Watcher_Linux", "error in select()");
		}
		QString NotRunningThread() {
			return QCoreApplication::translate("Watcher_Linux", "Not running thread");
		}
		QString AlreadyRunning() {
			return QCoreApplication::translate("Watcher_Linux", "Already running");
		}
	}
	namespace Watcher_Win {
		QString DragHereToAppend() {
			return QCoreApplication::translate("Watcher_Win", "Drag cursor from here to target window...");
		}
	}
}
