#pragma once
#include <QWidget>
#include <memory>

namespace Ui {
	class rectwindow;
}
namespace dg {
	class CellBoard;
	struct DomainSet;
	class rectwindow :
		public QWidget
	{
		Q_OBJECT
		private:
			std::shared_ptr<Ui::rectwindow>	_ui;
		public slots:
			void onGridChanged(const dg::DomainSet& ds, const dg::CellBoard& qm, size_t qs);
		public:
			explicit rectwindow(QWidget* parent=nullptr);
	};
}
