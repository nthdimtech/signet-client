#ifndef NOTETEXT_H
#define NOTETEXT_H

#include <QLabel>

class noteText : public QLabel
{
	Q_OBJECT
public:
	noteText(const QString &name);
	~noteText();
};

#endif
