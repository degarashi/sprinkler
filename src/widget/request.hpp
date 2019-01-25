#pragma once
#include "../place/param.hpp"
#include <QWidget>
#include <memory>

namespace dg { namespace widget {
	class EntrySliderI;
	class Request :
		public QWidget,
		private place::Param
	{
		Q_OBJECT
		private:
			EntrySliderI	*_avgImage;
		public slots:
			void setAvgImage(size_t n);
		signals:
			void avgChanged(size_t m);
		public:
			explicit Request(QWidget* parent=nullptr);
			const place::Param& param() const noexcept;
	};
}}
