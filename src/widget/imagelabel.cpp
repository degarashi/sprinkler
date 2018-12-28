#include "imagelabel.hpp"
#include "colorframe.hpp"
#include "../aux.hpp"
#include "../dbimage_if.hpp"
#include "../dbtag_if.hpp"
#include "../taginput.hpp"
#include "../sprinkler.hpp"
#include <QImageReader>
#include <QLabel>
#include <QMoveEvent>
#include <QMenu>

namespace dg { namespace widget {
	ImageLabel::ImageLabel(const ImageId id, const QSize crop,
					const lubee::PointI ofs, const QSize resize,
					QMenu* ctrlMenu,
					const DBImage *const dbImage,
					const DBTag *const dbTag):
		Obstacle(nullptr, Qt::SplashScreen|Qt::FramelessWindowHint),
		_id(id),
		_label(new QLabel(this)),
		_frame(new ColorFrame(this)),
		_ctrlMenu(ctrlMenu),
		_offset{ofs.x, ofs.y},
		_dbImage(dbImage),
		_dbTag(dbTag)
	{
		const auto path = dbImage->getFullPath(id);
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
		_frame->setVisible(false);
		show();
		update();
	}
	void ImageLabel::mousePressEvent(QMouseEvent*) {
		emit clicked();
	}
	void ImageLabel::moveEvent(QMoveEvent* e) {
		// (ウィンドウマネージャが指定した位置にピッタリ置いてくれない関係で)位置が違っていたら補正する
		if(e->pos() != _offset) {
			move(_offset);
		}
	}
	void ImageLabel::contextMenuEvent(QContextMenuEvent* e) {
		if(e->modifiers() & Qt::ControlModifier) {
			// 予め設定されたCtrlメニューを出す
			_ctrlMenu->popup(e->globalPos());
		} else {
			// ILinkタグを表示
			sprinkler.showTagMenu(_id, e->globalPos());
		}
	}
}}
