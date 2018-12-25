#include <QObject>

class QTimer;
class QScreen;
namespace dg {
	class QtWNotifier : public QObject {
		Q_OBJECT
		private:
			QTimer*		_delayTimer;
		public slots:
			void onScreenAdded(QScreen* scr);
			void onScreenRemoved(QScreen* scr);
			void onScreenGeometryChanged();
		signals:
			void onQtGeometryChanged();
		public:
			QtWNotifier(size_t delayMS = 200, QObject* parent=nullptr);
			bool eventFilter(QObject* obj, QEvent* e) override;
	};
}
