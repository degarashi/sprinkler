#pragma once
#include "geom_restore.hpp"
#include <QMainWindow>

namespace dg::widget {
	class GeomRestore_MW :
		public GeomRestore<QMainWindow>
	{
		Q_OBJECT
		private:
			using base_t = GeomRestore<QMainWindow>;
		protected:
			void _onVisibilityChanged(bool b) override;
		signals:
			void onVisibilityChanged(bool b);
		public:
			using base_t::base_t;
	};
}
