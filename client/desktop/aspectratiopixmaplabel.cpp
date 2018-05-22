#include "aspectratiopixmaplabel.h"

AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent) :
	QLabel(parent)
{
	this->setMinimumSize(1,1);
	setScaledContents(false);
}

void AspectRatioPixmapLabel::setPixmap ( const QPixmap & p)
{
	pix = p;
	QLabel::setPixmap(scaledPixmap(this->size()));
}

QPixmap AspectRatioPixmapLabel::scaledPixmap(QSize sz) const
{
	return pix.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QSize AspectRatioPixmapLabel::sizeHint() const
{
	int h = this->height();
	return QSize(h, h);
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent * e)
{
	if(!pix.isNull())
		QLabel::setPixmap(scaledPixmap(e->size()));
}
