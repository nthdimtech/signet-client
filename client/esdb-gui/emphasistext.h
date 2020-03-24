#ifndef EMPHASISTEXT_H
#define EMPHASISTEXT_H

#include <QLabel>

class emphasisText : public QLabel
{
	Q_OBJECT
public:
	emphasisText(const QString &name);
	~emphasisText();
};

#endif
