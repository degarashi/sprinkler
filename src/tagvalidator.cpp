#include "tagvalidator.hpp"
#include "dbtag_if.hpp"

namespace dg {
	TagValidator::TagValidator(const DBTag* tag, QObject *const parent):
		QValidator(parent),
		_dbTag(tag)
	{}
	void TagValidator::fixup(QString& input) const {
		// 前方一致でヒットした件数が1の時だけ補完する
		const auto tag = _dbTag->enumTagForwardMatch(input);
		if(tag.size() == 1) {
			input = _dbTag->getTagName(tag.front());
		}
	}
	TagValidator::State TagValidator::validate(QString& input, int& pos) const {
		Q_UNUSED(pos)
		const auto tag = _dbTag->enumTagForwardMatch(input);
		if(tag.empty()) {
			return State::Invalid;
		}
		for(const auto id : tag) {
			if(_dbTag->getTagName(id) == input) {
				return State::Acceptable;
			}
		}
		return State::Intermediate;
	}
}
