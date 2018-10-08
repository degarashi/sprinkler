#include "quantizer.hpp"
#include "aux.hpp"
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
			auto qm = _makeQuantizedMap();
			if(v.qmap != qm) {
				v.qmap = std::move(qm);
			}
			emit onGridChanged(_dset, v.qmap, _qsize);
		}),
		_qsize(qs)
	{}
	void Quantizer::_collectRects() {
		// 仮想デスクトップサイズ
		_dset.vscr = qApp->screens()[0]->virtualGeometry();
		// スクリーン矩形
		{
			auto& rv = _dset.rv[DomainType::Screen];
			rv.clear();
			for(const QScreen *const s : qApp->screens()) {
				rv.emplace_back(dg::Rect_Name{
					s->geometry(),
					s->name()
				});
			}
		}
		// Qtが管理するウィンドウ
		{
			auto& rv = _dset.rv[DomainType::Qt];
			rv.clear();
			for(const QWindow *const w : qApp->topLevelWindows()) {
				if(QVariant v = w->property("obstacle");
					!v.isValid() || !v.toBool())
				{
					continue;
				}
				if(w->isExposed() && w->isVisible()) {
					rv.emplace_back(dg::Rect_Name{
						w->frameGeometry(),
						w->objectName()
					});
				}
			}
		}
		// (監視対象のウィンドウはonRectChanged()でセットする)
	}
	CellBoard Quantizer::_makeQuantizedMap() {
		QRegion obstacle;
		const lubee::SizeI size(
			_dset.vscr.width() / _qsize,
			_dset.vscr.height() / _qsize
		);
		obstacle += QRect(0, 0, size.width, size.height);
		// スクリーン矩形
		for(auto& r : _dset.rv[DomainType::Screen]) {
			obstacle -= QuantifyS(r.rect, _qsize);
		}
		// Qtが管理するウィンドウ
		for(auto& r : _dset.rv[DomainType::Qt]) {
			QRegion reg(r.rect);
			obstacle += QuantifyS(reg.boundingRect(), _qsize);
		}
		// 監視対象のウィンドウ
		for(auto& r : _dset.rv[DomainType::Other]) {
			QRegion reg(r.rect);
			obstacle += QuantifyS(reg.boundingRect(), _qsize);
		}

		std::vector<lubee::RectI> obs;
		for(auto& o : obstacle)
			obs.emplace_back(ToLRect(o));
		return CellBoard(obs.data(), obs.size(), size);
	}
	void Quantizer::onRectChanged(dg::Rect_NameV rect) {
		qDebug() << "Quantizer: onRectChanged";
		_dset.rv[DomainType::Other] = std::move(rect);
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
