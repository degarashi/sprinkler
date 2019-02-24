#include "../file.hpp"
#include <QString>
#include <fileapi.h>
#include <iostream>

namespace dg {
	bool IsSymbolicLink(const QString& path) {
		const auto attr = GetFileAttributes(
			path.toLocal8Bit().toStdString().c_str()
		);
		constexpr auto Flag = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT;
		return (attr & Flag) == Flag;
	}
}
