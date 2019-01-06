#pragma once
#include "../place/param.hpp"
#include <QWidget>
#include <memory>

namespace dg { namespace widget {
	class EntrySliderD;
	class EntrySliderI;
	class Request :
		public QWidget,
		private place::Param
	{
		Q_OBJECT
		private:
			EntrySliderI	*_avgImage,
							*_samp;
		public slots:
			void setAvgImage(size_t n);
			void setNSample(size_t n);
		signals:
			void avgChanged(size_t m);
			void nSampleChanged(size_t n);
		public:
			explicit Request(QWidget* parent=nullptr);
			const place::Param& param() const noexcept;
	};
}}
