#include "quantizer.hpp"
#include "aux.hpp"
#include "widget/obstacle.hpp"
#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <QDebug>

namespace dg {
	namespace {
		lubee::RectI ToLRect(const QRect& r) {
			return {
				r.left(),
				r.right()+1,
				r.top(),
				r.bottom()+1
			};
		}
	}
	Quantizer::Value::Value():
		qmap({1,1})
	{}
	Quantizer::Quantizer(const size_t qs, QObject* parent):
		QObject(parent),
		_value([this](Value& v){
			_collectRects();
			auto qm = _makeCellBoard();
			if(v.qmap != qm) {
				v.qmap = std::move(qm);
				emit onGridChanged(_dset, v.qmap, _qsize);
			}
		}),
		_qsize(qs)
	{}
	void Quantizer::_collectRects() {
		// 仮想デスクトップサイズ
		_dset.vscr = qApp->screens()[0]->virtualGeometry();
		// スクリーン矩形
		{
			auto& rv = _dset.domain.screen;
			rv.clear();
			for(const QScreen *const s : qApp->screens()) {
				rv.emplace_back(Domain{
					s->geometry(),
					s->name()
				});
			}
		}
		// Qtが管理するウィンドウ
		{
			auto& rv = _dset.domain.qt;
			rv.clear();
			for(const QWindow *const w : qApp->topLevelWindows()) {
				if(!widget::ObstacleBase::IsObstacle(w))
					continue;
				if(w->isExposed() &&
					w->isVisible() &&
					w->windowState() != Qt::WindowState::WindowMinimized)
				{
					rv.emplace_back(Domain{
						w->frameGeometry(),
						w->title()
					});
				}
			}
		}
		// (監視対象のウィンドウはonWatchedRectChanged()でセットする)
	}
	CellBoard Quantizer::_makeCellBoard() {
		QRegion obstacle;
		const lubee::SizeI size(
			_dset.vscr.width() / _qsize,
			_dset.vscr.height() / _qsize
		);
		obstacle += QRect(0, 0, size.width, size.height);
		// スクリーン矩形
		for(auto& r : _dset.domain.screen) {
			obstacle -= QuantifyS(r.rect, _qsize);
		}
		// Qtが管理するウィンドウ
		for(auto& r : _dset.domain.qt) {
			QRegion reg(r.rect);
			obstacle += QuantifyS(reg.boundingRect(), _qsize);
		}
		// 監視対象のウィンドウ
		for(auto& r : _dset.domain.other) {
			QRegion reg(r.rect);
			obstacle += QuantifyS(reg.boundingRect(), _qsize);
		}

		std::vector<lubee::RectI> obs;
		for(auto& o : obstacle)
			obs.emplace_back(ToLRect(o));
		return CellBoard(obs.data(), obs.size(), size);
	}
	void Quantizer::onWatchedRectChanged(const DomainV& rect) {
		qDebug() << "Quantizer: onWatchedRectChanged";
		_dset.domain.other = rect;
		_value.setDirty();
		_value.get();
	}
	void Quantizer::onQtGeometryChanged() {
		qDebug() << "Quantizer: onQtGeometryChanged";
		_value.setDirty();
		_value.get();
	}
	const DomainSet& Quantizer::domainSet() const noexcept {
		return _dset;
	}
	const CellBoard& Quantizer::qmap() const noexcept {
		return _value.get().qmap;
	}
	size_t Quantizer::quantizedSize() const noexcept {
		return _qsize;
	}
}
