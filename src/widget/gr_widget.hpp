#pragma once
#include "geom_restore.hpp"
#include <QWidget>

namespace dg::widget {
	class GeomRestore_Widget :
		public GeomRestore<QWidget>
	{
		Q_OBJECT
		private:
			using base_t = GeomRestore<QWidget>;
		protected:
			void _onVisibilityChanged(bool b) override;
		signals:
			void onVisibilityChanged(bool b);
		public:
			using base_t::base_t;
	};
}
