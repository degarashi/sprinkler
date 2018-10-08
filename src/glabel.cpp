#include "glabel.hpp"
#include "aux.hpp"
#include <QContextMenuEvent>
#include <QMenu>
#include <QTimer>
#include <QPixmap>
#include <QLabel>
#include <QPainter>
#include <QImageReader>

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
					const bool keep, QMenu* ctrlMenu):
		Obstacle(nullptr, Qt::SplashScreen|Qt::FramelessWindowHint),
		_label(new QLabel(this)),
		_frame(new GFrame(this)),
		_path(path),
		_keep(keep),
		_ctrlMenu(ctrlMenu),
		_offset{ofs.x, ofs.y}
	{
		const auto readImg = [&path](const QSize s){
			QImageReader reader(path);
			reader.setScaledSize(AspectKeepScale(s, reader.size()));
			return reader.read();
		};
		const QImage img = readImg(resize).copy({{0,0}, crop});
		const QPixmap pix = QPixmap::fromImage(img);
		_label->setPixmap(pix);
		move(ofs.x, ofs.y);
		this->resize(_label->sizeHint());
		const QSize lbs = _label->sizeHint();
		_frame->setGeometry(0,0, lbs.width(), lbs.height());
		_frame->setVisible(_getChecked());
		show();
		update();
	}
	bool GLabel::_getChecked() const {
		return _keep;
	}
	void GLabel::mousePressEvent(QMouseEvent*) {
		emit clicked();
	}
	void GLabel::moveEvent(QMoveEvent* e) {
		// (ウィンドウマネージャが指定した位置にピッタリ置いてくれない関係で)位置が違っていたら補正する
		if(e->pos() != _offset) {
			move(_offset);
		}
	}
	void GLabel::contextMenuEvent(QContextMenuEvent* e) {
		if(e->modifiers() & Qt::ControlModifier) {
			_ctrlMenu->popup(e->globalPos());
		} else {
			QMenu menu(this);
			QAction* act = menu.addAction(tr("Keep"));
			act->setShortcut(QKeySequence(tr("k", "Keep")));
			act->setCheckable(true);
			act->setChecked(_getChecked());
			connect(act, &QAction::toggled, this, [this](const bool b){
				_keep = !_keep;
				emit keepChanged(_path, _keep);
			});
			menu.exec(e->globalPos());
		}
	}
	void GLabel::showLabelFrame(const bool b) {
		_frame->setVisible(b && _getChecked());
	}
	const QString& GLabel::path() const noexcept {
		return _path;
	}
	const QPixmap* GLabel::pixmap() const {
		return _label->pixmap();
	}
	void GLabel::setKeep(const QString& path, bool b) {
		if(_path == path) {
			_keep = b;
			emit keepChanged(_path, _keep);
		}
	}
}
