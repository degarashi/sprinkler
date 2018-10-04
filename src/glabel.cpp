#include "glabel.hpp"
#include <QContextMenuEvent>
#include <QMenu>
#include <QTimer>
#include <QPixmap>

Q_DECLARE_METATYPE(dg::KeepData)
namespace dg {
	GLabel::GLabel(const QString& path, const QSize crop,
					const lubee::PointI ofs, const QSize resize,
					const QModelIndex& index):
		QLabel(nullptr, Qt::SplashScreen|Qt::FramelessWindowHint),
		_path(path),
		_index(index)
	{
		QImage img(path);

		const_cast<QAbstractItemModel*>(index.model())->setData(
			index,
			QPixmap::fromImage(img.scaled({64,64}, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
			Qt::DecorationRole
		);
		img = img.scaled(resize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		img = img.copy({{0,0}, crop});
		const QPixmap pix = QPixmap::fromImage(img);
		setPixmap(pix);
		move(ofs.x, ofs.y);
		// ウィンドウマネージャが指定した位置にピッタリ置いてくれない関係で若干のディレイを入れる
		QTimer::singleShot(1, this, [this, ofs](){
			move(ofs.x, ofs.y);
		});
		show();
		update();
	}
	void GLabel::mousePressEvent(QMouseEvent*) {
		emit clicked();
	}
	void GLabel::contextMenuEvent(QContextMenuEvent* e) {
		if(!_index.isValid())
			return;
		QMenu menu(this);
		QAction* act = menu.addAction(tr("Keep"));
		act->setShortcut(QKeySequence(tr("k", "Keep")));
		act->setCheckable(true);
		act->setChecked(_index.data(Qt::UserRole).value<KeepData>().keep);
		connect(act, &QAction::toggled, this, [this](const bool b){
			if(_index.isValid()) {
				auto kp = _index.model()->data(_index, Qt::UserRole).value<KeepData>();
				if(kp.keep != b) {
					kp.keep = b;
					const_cast<QAbstractItemModel*>(_index.model())->setData(_index, QVariant::fromValue(kp), Qt::UserRole);
				}
			}
		});
		menu.exec(e->globalPos());
	}
}
