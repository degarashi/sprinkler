#include "imagelabel.hpp"
#include "colorframe.hpp"
#include "../aux.hpp"
#include "../dbimage_if.hpp"
#include "../dbtag_if.hpp"
#include "../taginput.hpp"
#include "../sprinkler.hpp"
#include "../q_rs_op.hpp"
#include <QImageReader>
#include <QLabel>
#include <QMoveEvent>
#include <QMenu>

namespace dg { namespace widget {
	ImageLabel::ImageLabel(
		const ImageId id,
		const lubee::RectI& rect,
		QMenu* ctrlMenu,
		const DBImage *const dbImage,
		const DBTag *const dbTag
	):
		Obstacle(nullptr, Qt::SplashScreen|Qt::FramelessWindowHint),
		_id(id),
		_label(new QLabel(this)),
		_frame(new ColorFrame(this)),
		_ctrlMenu(ctrlMenu),
		_offset(ToQPoint(rect.offset())),
		_dbImage(dbImage),
		_dbTag(dbTag)
	{
		{
			const auto calcSize = [id, &rect, dbImage](){
				const auto& imgInfo = dbImage->getImageInfo(id);
				// 画像をブロック枠サイズにピッタリ合わせる
				// 置く予定のスペース
				const lubee::SizeI space = rect.size();
				// 補正後の画像サイズ
				const QSize adjSize = AspectKeepScale(
					ToQSize(space),
					ToQSize(imgInfo.size)
				);
				Q_ASSERT(adjSize.width() <= space.width
						&& adjSize.height() <= space.height);
				QPoint ofs;
				if(space.width == adjSize.width()) {
					// 縦のオフセット調整
					ofs.setX(0);
					ofs.setY(space.height/2 - adjSize.height()/2);
				} else {
					Q_ASSERT(space.height == adjSize.height());
					// 横のオフセット調整
					ofs.setX(space.width/2 - adjSize.width()/2);
					ofs.setY(0);
				}
				return QRect(ofs, adjSize);
			};
			const auto readImg = [id, dbImage](const QSize s){
				const auto path = dbImage->getFullPath(id);
				QImageReader reader(path);
				reader.setScaledSize(s);
				return reader.read();
			};
			const auto adj_r = calcSize();
			_label->setPixmap(QPixmap::fromImage(readImg(adj_r.size())));
			_label->move(adj_r.topLeft());
		}
		// 色付きフレーム枠の初期化
		_frame->setVisible(false);
		_frame->setGeometry(
			0,0,
			rect.size().width, rect.size().height
		);
		// 背景色を黒に変更
		{
			auto pal = this->palette();
			pal.setColor(QPalette::Window, "black");
			setPalette(pal);
		}
		move(_offset);
		resize(ToQSize(rect.size()));
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
			sprinkler.showImageContextMenu(_id, e->globalPos());
		}
	}
}}
