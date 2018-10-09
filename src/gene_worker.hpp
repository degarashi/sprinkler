#pragma once
#include "gene_param.hpp"
#include "place_result.hpp"
#include <QObject>

namespace dg {
	class CellBoard;
	class GeneWorker :
		public QObject
	{
		Q_OBJECT

		public slots:
			void calcArea(const dg::RequestParam& param, const dg::CellBoard& initial,
							const dg::ImageSet& imgkeep, dg::ImageSet img,
							size_t qs);
		signals:
			void geneResult(const dg::PlaceV& place);
			void onProgress(int p);
	};
}
