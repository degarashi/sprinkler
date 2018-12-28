#include "tagselector.hpp"
#include "../tagselect_model.hpp"
#include "../tagvalidator.hpp"
#include "../dbtag_if.hpp"
#include "../database_sig.hpp"
#include "ui_tagselector.h"
#include <QSqlTableModel>

namespace dg::widget {
	TagSelector::TagSelector(QWidget* parent):
		QWidget(parent),
		_ui(new Ui::TagSelector),
		_dbTag(nullptr)
	{
		_ui->setupUi(this);
	}
	void TagSelector::init(const DBTag *const tag, const DatabaseSignal *const sig) {
		Q_ASSERT(!_dbTag);
		_dbTag = tag;

		_tagCand = tag->makeTagModel(this);
		_tagCand->setSort(1, Qt::AscendingOrder);
		_ui->candTag->setModel(_tagCand);
		_ui->candTag->setModelColumn(1);

		_tagSelected = new TagSelectModel(tag, sig, this);
		_ui->selectTag->setModel(_tagSelected);
		_ui->selectTag->setModelColumn(0);

		const auto refl = [this](auto...){ _refreshCount(); };
		connect(_tagSelected, &QAbstractItemModel::rowsRemoved, this, refl);
		connect(_tagSelected, &QAbstractItemModel::rowsInserted, this, refl);
		connect(_tagSelected, &QAbstractItemModel::modelReset, this, refl);
		connect(sig, &DatabaseSignal::endResetImage, this, refl);
		connect(sig, &DatabaseSignal::endResetLink, this, refl);
		_refreshCount();

		_ui->leTag->setValidator(new TagValidator(tag, this));
		connect(_ui->leTag, &QLineEdit::textChanged,
			this, [this](const QString& text){
				// 前方一致で候補を検索
				_tagCand->setFilter(QString("name LIKE '%1%'").arg(text));
			});
		QMetaObject::invokeMethod(_ui->leTag, "textChanged", Q_ARG(QString, ""));

		connect(_tagSelected, &QAbstractItemModel::modelReset,
			this, [this](){
				emit changed(_tagSelected->getArray());
		});
	}
	void TagSelector::_refreshCount() const {
		// タグ番号配列を取得
		const auto tags = _tagSelected->getArray();
		const auto [total, shown] = _dbTag->countImageByTag(tags);
		// ANDで検索
		_ui->lbCount->setText(
			QString(tr("%1 tags selected\t(%2 images found, %3 images already shown)"))
			.arg(tags.size())
			.arg(total)
			.arg(shown)
		);
	}
	void TagSelector::onAdd() {
		QString tagName;
		{
			const auto sel = _ui->candTag->selectionModel()->selectedIndexes();
			if(sel.size() == 1) {
				tagName = sel[0].data().toString();
			} else {
				const auto le = _ui->leTag->text();
				if(!le.isEmpty()) {
					if(_dbTag->getTagId(le)) {
						tagName = le;
						_ui->leTag->selectAll();
					}
				}
			}
		}
		if(!tagName.isEmpty()) {
			const auto tagId = _dbTag->getTagId(tagName);
			Q_ASSERT(tagId);
			_tagSelected->add(*tagId);
		}
	}
	void TagSelector::onRem() {
		const auto sl = _ui->selectTag->selectionModel()->selectedRows();
		std::vector<int> idxv;
		for(const auto& idx : sl)
			idxv.emplace_back(idx.row());
		std::sort(idxv.begin(), idxv.end());
		std::reverse(idxv.begin(), idxv.end());
		for(const auto idx : idxv)
			_tagSelected->rem(idx);
	}
	TagIdV TagSelector::getArray() const {
		return _tagSelected->getArray();
	}
}
