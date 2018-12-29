#include "version.hpp"
#include <QString>
#include <QSettings>

namespace dg {
	std::string Version::asString() const {
		return
			std::to_string(major)
			+ "."
			+ std::to_string(minor)
			+ "."
			+ std::to_string(release)
		;
	}
	namespace {
		const QString
			ver_major("major"),
			ver_minor("minor"),
			ver_release("release");
	}
	VersionOpt Version::FromSettings(const QSettings& s) {
		bool ok[3];
		const Version ret{
			s.value(ver_major).toUInt(ok),
			s.value(ver_minor).toUInt(ok +1),
			s.value(ver_release).toUInt(ok +2)
		};
		if(!(ok[0] & ok[1] & ok[2]))
			return std::nullopt;
		return ret;
	}
	void Version::writeSettings(QSettings& s) const {
		s.setValue(ver_major, major);
		s.setValue(ver_minor, minor);
		s.setValue(ver_release, release);
	}
	Version Version::ThisVersion() noexcept {
		Version ret;
		ret.major = 0;
		ret.minor = 1;
		ret.release = 1;
		return ret;
	}
	bool Version::operator < (const Version& v) const noexcept {
		return std::lexicographical_compare(
			array.begin(), array.end(),
			v.array.begin(), v.array.end()
		);
	}
	bool Version::operator == (const Version& v) const noexcept {
		return array == v.array;
	}
}
