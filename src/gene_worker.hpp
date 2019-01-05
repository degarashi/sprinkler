#pragma once
#include <QObject>

namespace dg {
	class CellBoard;
	namespace place {
		struct Selected;
		using SelectedV = std::vector<Selected>;
		struct Result;
		using ResultV = QVector<Result>;
	}
	class GeneWorker :
		public QObject
	{
		Q_OBJECT
		private:
			bool	_abort;

		public slots:
			void sprinkle(
				const dg::CellBoard& initial,
				const dg::place::SelectedV&	selected,
				size_t qs
			);
			void abort();
		signals:
			void sprinkleResult(const dg::place::ResultV& result);
			void sprinkleProgress(int p);
			void sprinkleAbort();
	};
}
