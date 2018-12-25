#include "domain.hpp"
#include "cache.hpp"
#include "histgram/src/cell.hpp"
#include <QObject>

namespace dg {
	// ウィンドウ移動やスクリーン追加削除の通知を受けてその領域情報を量子化
	class Quantizer :
		public QObject
	{
		Q_OBJECT
		private:
			DomainSet		_dset;
			struct Value {
				CellBoard	qmap;
				Value();
			};
			Cache<Value>	_value;
			size_t			_qsize;

			void _collectRects();
			CellBoard _makeCellBoard();

		public slots:
			void onWatchedRectChanged(const dg::DomainV& rect);
			void onQtGeometryChanged();
		signals:
			void onGridChanged(const dg::DomainSet& ds, const dg::CellBoard& qm, size_t qs);
		public:
			Quantizer(const Quantizer&) = delete;
			Quantizer(Quantizer&&) = delete;
			Quantizer(size_t qs, QObject* parent=nullptr);
			const DomainSet& domainSet() const noexcept;
			const CellBoard& qmap() const noexcept;
			size_t quantizedSize() const noexcept;
	};
}
