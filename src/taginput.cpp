#include "taginput.hpp"
#include "ui_taginput.h"
#include "dbtag_if.hpp"
#include <QPushButton>

namespace dg {
	TagInput::TagInput(
		DatabaseSignal* sig,
		DBTag* dbTag,
		QWidget *parent
	):
		QDialog(parent),
		_ui(new Ui::TagInput)
	{
		_ui->setupUi(this);
		_ui->tagSelector->init(dbTag, sig);
		connect(_ui->rbExisting, &QRadioButton::toggled,
				this, [this](const bool b){
					_setExistingMode(b);
				});
		connect(_ui->tagSelector, &widget::TagSelector::changed,
				this, [this](const TagIdV& tag){
					// Existingの時しか来ないと仮定
					_setOKEnabled(!tag.empty());
				});
		connect(_ui->leTag, &QLineEdit::textChanged,
				this, [this](const QString& str){
					// Newの時しか来ないと仮定
					// 空文字列はタグとして認められない
					_setOKEnabled(!str.isEmpty());
				});
		connect(this, &QDialog::accepted,
				this, [this, dbTag](){
					TagIdV sel;
					if(_getExistingMode()) {
						sel = _ui->tagSelector->getArray();
					} else {
						const QString name = _ui->leTag->text();
						// 既存のタグが存在すればそれを使用 (無ければ新しく作る)
						sel = {dbTag->makeTag(name)};
					}
					emit accepted(sel);
				});
		_setOKEnabled(false);
		_setExistingMode(true);
	}
	void TagInput::_setExistingMode(const bool b) {
		_ui->tagSelector->setEnabled(b);
		_ui->leTag->setEnabled(!b);
	}
	bool TagInput::_getExistingMode() const {
		return _ui->rbExisting->isChecked();
	}
	void TagInput::_setOKEnabled(const bool b) {
		_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(b);
	}
}
