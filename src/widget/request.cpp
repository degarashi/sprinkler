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
		_max(new EntrySliderD),
		_min(new EntrySliderD),
		_samp(new EntrySliderI)
	{
		auto* l = new QVBoxLayout(this);

		_max->setRange({0.1f, 1.f});
		_max->setName(tr("Max"));
		_max->setPageStep(0.1f);
		connect(_max, &EntrySliderD::valueChanged, this, [this](const float v){
			setMax(v);
		});
		setMax(0.6f);
		l->addWidget(_max);

		_min->setRange({0.1f, 1.f});
		_min->setName(tr("Min"));
		_min->setPageStep(0.1f);
		connect(_min, &EntrySliderD::valueChanged, this, [this](const float v){
			setMin(v);
		});
		setMin(0.3f);
		l->addWidget(_min);

		_samp->setRange({1, 8});
		_samp->setName(tr("Samp"));
		_samp->setPageStep(2);
		connect(_samp, &EntrySliderI::valueChanged, this, [this](const int v){
			setNSample(v);
		});
		setNSample(4);
		l->addWidget(_samp);
	}
	void Request::setMax(const float m) {
		if(CompareAndSet(sizeRange.to, m)) {
			setMin(std::min(sizeRange.from, sizeRange.to));
			_max->setValue(m);
			emit maxChanged(m);
		}
	}
	void Request::setMin(const float m) {
		if(CompareAndSet(sizeRange.from, m)) {
			setMax(std::max(sizeRange.to, sizeRange.from));
			_min->setValue(m);
			emit minChanged(m);
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
