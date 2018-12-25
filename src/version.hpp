#pragma once
#include <cstdint>
#include <string>
#include <array>
#include <optional>

class QSettings;
namespace dg {
	struct Version;
	using VersionOpt = std::optional<Version>;
	struct Version {
		using Num = uint32_t;
		union {
			struct {
				Num		major,
						minor,
						release;
			};
			std::array<Num, 3>	array;
		};
		std::string asString() const;
		static VersionOpt FromSettings(const QSettings& s);
		static Version ThisVersion() noexcept;
		void writeSettings(QSettings& s) const;

		bool operator < (const Version& v) const noexcept;
		bool operator == (const Version& v) const noexcept;
	};
}
