#pragma once
#include <QLabel>

namespace dg { namespace widget {
	class CurIndicator :
		public QLabel
	{
		Q_OBJECT
		private:
			size_t		_cur=0,
						_num=0;
			void _makeText();
		public:
			using QLabel::QLabel;
		public slots:
			void onSprinkleCounterChanged(size_t cur, size_t num);
	};
}}
