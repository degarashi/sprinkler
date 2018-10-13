#pragma once
#include <QWidget>
#include <memory>
#include "request_param.hpp"

namespace dg {
	class MySliderD;
	class MySliderI;
	class Request :
		public QWidget,
		private RequestParam
	{
		Q_OBJECT
		private:
			MySliderD	*_max,
						*_min;
			MySliderI	*_samp;
		public slots:
			void setMin(float m);
			void setMax(float m);
			void setNSample(size_t n);
		signals:
			void minChanged(float m);
			void maxChanged(float m);
			void nSampleChanged(size_t n);
		public:
			explicit Request(
				QWidget* parent = nullptr
			);
			const RequestParam& param() const noexcept;
	};
}
