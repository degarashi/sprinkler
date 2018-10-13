#pragma once
#include <QWidget>
#include <QSlider>
#include <memory>

namespace Ui {
	class MySlider;
}
namespace dg {
	// 項目名, スライダー, 数値を並べたものを表示
	class MySlider :
		public QWidget
	{
		Q_OBJECT
		private:
			using Ui_S = std::shared_ptr<Ui::MySlider>;
			Ui_S			_ui;
			QString			_suffix;
		protected:
			virtual QString _makeValueString(int value) const = 0;
			virtual void _onValueChanged() = 0;
			QSlider* _slider() const;
		public:
			explicit MySlider(QWidget* parent=nullptr);
			void setName(const QString& name);
			void setSuffix(const QString& sfx);
	};
}
