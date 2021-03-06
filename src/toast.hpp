#pragma once
#include <QWidget>
#include <memory>

namespace Ui {
	class Toast;
}
namespace dg {
	class Toast :
		public QWidget
	{
		Q_OBJECT
		Q_PROPERTY(float alpha READ alpha WRITE setAlpha)
		private:
			using base_t = QWidget;
			std::shared_ptr<Ui::Toast> _ui;
			void setAlpha(float val);
			float alpha();
		public:
			struct Icon {
				enum e {
					Information,
					Warning,
					Critical,
					Question,
					_Num
				};
			};

			Toast(
				Icon::e	iconType,
				const QString& title,
				const QString& msg,
				int fadeInMS,
				int durationMS,
				int fadeOutMS
			);
	};
}
