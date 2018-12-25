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
			EntrySliderD	*_max,
							*_min;
			EntrySliderI	*_samp;
		public slots:
			void setMin(float m);
			void setMax(float m);
			void setNSample(size_t n);
		signals:
			void minChanged(float m);
			void maxChanged(float m);
			void nSampleChanged(size_t n);
		public:
			explicit Request(QWidget* parent=nullptr);
			const place::Param& param() const noexcept;
	};
}}
