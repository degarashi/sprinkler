#include "../file.hpp"
#include <QFileInfo>

namespace dg {
	bool IsSymbolicLink(const QString& path) {
		// そのままQFileInfoのisSymLinkを使う
		return QFileInfo(path).isSymLink();
	}
}
