#include "request.hpp"
#include "entryslider_d.hpp"
#include "entryslider_i.hpp"
#include "../toml_settings.hpp"
#include <QVBoxLayout>

namespace dg { namespace widget {
	namespace {
		template <class T0, class T1>
		bool CompareAndSet(T0& dst, const T1& val) {
			const bool diff = dst != val;
			dst = val;
			return diff;
		}
		namespace param {
			#define DefP(name, ent, type) \
				Define_TomlSet("Request", name, ent, type)
			DefP(AvgImage_Min, "avg_image_min", size_t)
			DefP(AvgImage_Max, "avg_image_max", size_t)
			DefP(AvgImage_Default, "avg_image_default", size_t)
			#undef DefP
		}
	}
	Request::Request(
		QWidget* parent
	):
		QWidget(parent),
		_avgImage(new EntrySliderI)
	{
		auto* l = new QVBoxLayout(this);

		_avgImage->setRange({param::AvgImage_Min(), param::AvgImage_Max()});
		_avgImage->setName(tr("Avg_Image"));
		_avgImage->setPageStep(1);
		connect(_avgImage, &EntrySliderI::valueChanged, this, [this](const int v){
			setAvgImage(size_t(v));
		});
		setAvgImage(param::AvgImage_Default());
		l->addWidget(_avgImage);
	}
	void Request::setAvgImage(const size_t n) {
		if(CompareAndSet(avgImage, n)) {
			_avgImage->setValue(int(n));
			emit avgChanged(n);
		}
	}
	const place::Param& Request::param() const noexcept {
		return *this;
	}
}}
