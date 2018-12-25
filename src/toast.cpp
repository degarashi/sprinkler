#include "toast.hpp"
#include "ui_toast.h"
#include <QTimer>
#include <QPropertyAnimation>
#include <QApplication>
#include <QScreen>
#include <QStyle>

namespace dg {
	namespace {
		const QStyle::StandardPixmap c_icon[Toast::Icon::_Num] = {
			QStyle::SP_MessageBoxInformation,
			QStyle::SP_MessageBoxWarning,
			QStyle::SP_MessageBoxCritical,
			QStyle::SP_MessageBoxQuestion,
		};
	}
	Toast::Toast(
		const Icon::e iconType,
		const QString& title,
		const QString& msg,
		const int fadeInMS,
		const int durationMS,
		const int fadeOutMS
	):
		base_t(nullptr, Qt::SplashScreen),
		_ui(new Ui::Toast)
	{
		_ui->setupUi(this);
		setAttribute(Qt::WA_TranslucentBackground);

		QIcon icon = QApplication::style()->standardIcon(c_icon[iconType]);
		_ui->lbIcon->setPixmap(icon.pixmap(icon.availableSizes().last()));
		_ui->lbIcon->setAlignment(Qt::AlignVCenter);
		_ui->lbTitle->setText(title);
		_ui->lbMessage->setText(msg);

		setWindowFlag(Qt::WindowStaysOnTopHint);
		setWindowFlag(Qt::FramelessWindowHint);

		QPropertyAnimation *animation = new QPropertyAnimation(this, "alpha", this);
		animation->setKeyValueAt(0, 0.f);
		animation->setKeyValueAt(1, 1.f);
		animation->setEasingCurve(QEasingCurve::OutCubic);
		animation->setDuration(fadeInMS);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		connect(
			animation, &QPropertyAnimation::finished,
			this, [fadeInMS, durationMS, fadeOutMS, this](){
				QTimer::singleShot(durationMS + fadeInMS, this, [fadeOutMS, this](){
					QPropertyAnimation *animation = new QPropertyAnimation(this, "alpha", this);
					animation->setKeyValueAt(0, 1.f);
					animation->setKeyValueAt(1, 0.f);
					animation->setEasingCurve(QEasingCurve::InQuad);
					animation->setDuration(fadeOutMS);
					animation->start(QAbstractAnimation::DeleteWhenStopped);
					connect(animation, &QPropertyAnimation::finished,
							this, [this](){
								deleteLater();
							});
				});
		});

		const QSize sz = size();
		const int MarginX = 32,
					MarginY = 16;
		const QRect rect = qApp->primaryScreen()->availableGeometry();
		move(
			rect.x() + rect.width() - sz.width() - MarginX,
			rect.y() + rect.height() - sz.height() - MarginY
		);
	}
	void Toast::setAlpha(const float val) {
		setWindowOpacity(val);
	}
	float Toast::alpha() {
		return windowOpacity();
	}
}
