#ifndef ASPECTRATIOPIXMAPLABEL_H
#define ASPECTRATIOPIXMAPLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

class AspectRatioPixmapLabel : public QLabel
{
	Q_OBJECT
public:
	explicit AspectRatioPixmapLabel(QWidget *parent = 0);

	virtual QSize sizeHint() const;
	QPixmap scaledPixmap(QSize sz) const;
public slots:
	virtual void setPixmap ( const QPixmap & );
	virtual void resizeEvent(QResizeEvent *);
private:
	QPixmap pix;
};

#endif // ASPECTRATIOPIXMAPLABEL_H
