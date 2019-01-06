#include "request.hpp"
#include "entryslider_d.hpp"
#include "entryslider_i.hpp"
#include <QVBoxLayout>

namespace dg { namespace widget {
	namespace {
		template <class T0, class T1>
		bool CompareAndSet(T0& dst, const T1& val) {
			const bool diff = dst != val;
			dst = val;
			return diff;
		}
	}
	Request::Request(
		QWidget* parent
	):
		QWidget(parent),
		_avgImage(new EntrySliderI),
		_samp(new EntrySliderI)
	{
		auto* l = new QVBoxLayout(this);

		_avgImage->setRange({4, 64});
		_avgImage->setName(tr("Avg_Image"));
		_avgImage->setPageStep(1);
		connect(_avgImage, &EntrySliderI::valueChanged, this, [this](const int v){
			setAvgImage(v);
		});
		setAvgImage(16);
		l->addWidget(_avgImage);

		_samp->setRange({1, 4});
		_samp->setName(tr("Samp"));
		_samp->setPageStep(1);
		connect(_samp, &EntrySliderI::valueChanged, this, [this](const int v){
			setNSample(v);
		});
		setNSample(1);
		l->addWidget(_samp);
	}
	void Request::setAvgImage(const size_t n) {
		if(CompareAndSet(avgImage, n)) {
			_avgImage->setValue(n);
			emit avgChanged(n);
		}
	}
	void Request::setNSample(const size_t n) {
		if(CompareAndSet(nSample, n)) {
			_samp->setValue(n);
			emit nSampleChanged(n);
		}
	}
	const place::Param& Request::param() const noexcept {
		return *this;
	}
}}
