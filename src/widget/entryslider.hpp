#pragma once
#include "ui_entryslider.h"
#include "lubee/src/range.hpp"
#include <memory>

namespace dg::widget {
	// 項目名, スライダー, 数値を並べたものを表示
	template <class Value>
	class EntrySlider :
		public QWidget
	{
		// Q_OBJECTは、テンプレートクラスに対応してないので敢えて記述しない
		private:
			using value_t = Value;
			using Ui_S = std::shared_ptr<Ui::EntrySlider>;
			Ui_S			_ui;
			QString			_suffix;
		protected:
			virtual int _toUi(value_t v) const = 0;
			virtual value_t _fromUi(int v) const = 0;
			virtual QString _makeValueString(value_t value) const = 0;
			virtual void _onValueChanged() = 0;
			QSlider* _slider() const {
				return _ui->slValue;
			}
		public:
			explicit EntrySlider(QWidget *parent=nullptr):
				QWidget(parent),
				_ui(new Ui::EntrySlider)
			{
				_ui->setupUi(this);
				_ui->lbValue->setFixedWidth(24);
				connect(
					_ui->slValue,
					&QSlider::valueChanged,
					this,
					[this](const int v){
						_ui->lbValue->setText(_makeValueString(_fromUi(v)) + _suffix);
						_onValueChanged();
					}
				);
			}
			void setName(const QString& name) {
				_ui->lbName->setText(name);
			}
			void setSuffix(const QString& sfx) {
				_suffix = sfx;
			}
			void setSingleStep(const value_t step) {
				_slider()->setSingleStep(std::max(1, _toUi(step)));
			}
			void setPageStep(const value_t step) {
				_slider()->setPageStep(std::max(1, _toUi(step)));
			}
			void setValue(const value_t v) {
				_slider()->setValue(_toUi(v));
			}
			void setRange(const lubee::RangeF r) {
				_slider()->setRange(_toUi(r.from), _toUi(r.to));
			}
	};
}
