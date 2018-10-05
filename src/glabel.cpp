#include "glabel.hpp"
#include <QContextMenuEvent>
#include <QMenu>
#include <QTimer>
#include <QPixmap>
#include <QLabel>
#include <QPainter>

Q_DECLARE_METATYPE(dg::KeepData)
namespace dg {
	namespace {
		constexpr int LineWidth = 3;
	}
	GFrame::GFrame(QWidget* parent):
		QWidget(parent)
	{
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_TranslucentBackground);
		setAttribute(Qt::WA_TransparentForMouseEvents);
	}
	void GFrame::paintEvent(QPaintEvent* e) {
		QPainter pt(this);
		const QSize s = size();
		QPen pen;
		pen.setCapStyle(Qt::FlatCap);
		pen.setJoinStyle(Qt::MiterJoin);
		pen.setWidth(LineWidth);
		pen.setColor(Qt::green);
		pt.setPen(pen);
		pt.drawRect(1, 1, s.width()-LineWidth, s.height()-LineWidth);
	}

	GLabel::GLabel(const QString& path, const QSize crop,
					const lubee::PointI ofs, const QSize resize,
					const QModelIndex& index, QMenu* ctrlMenu):
		QWidget(nullptr, Qt::SplashScreen|Qt::FramelessWindowHint),
		_label(new QLabel(this)),
		_frame(new GFrame(this)),
		_path(path),
		_index(index),
		_ctrlMenu(ctrlMenu)
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
		_label->setPixmap(pix);
		move(ofs.x, ofs.y);
		// ウィンドウマネージャが指定した位置にピッタリ置いてくれない関係で若干のディレイを入れる
		QTimer::singleShot(1, this, [this, ofs](){
			move(ofs.x, ofs.y);
		});
		this->resize(_label->sizeHint());
		const QSize lbs = _label->sizeHint();
		_frame->setGeometry(0,0, lbs.width(), lbs.height());
		_frame->setVisible(_getChecked());
		show();
		update();
	}
	bool GLabel::_getChecked() const {
		return _index.data(Qt::UserRole).value<KeepData>().keep;
	}
	void GLabel::mousePressEvent(QMouseEvent*) {
		emit clicked();
	}
	void GLabel::contextMenuEvent(QContextMenuEvent* e) {
		if(e->modifiers() & Qt::ControlModifier) {
			_ctrlMenu->popup(e->globalPos());
		} else {
			QMenu menu(this);
			if(!_index.isValid())
				return;
			QAction* act = menu.addAction(tr("Keep"));
			act->setShortcut(QKeySequence(tr("k", "Keep")));
			act->setCheckable(true);
			act->setChecked(_getChecked());
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
	void GLabel::showLabelFrame(const bool b) {
		_frame->setVisible(b && _getChecked());
	}
}