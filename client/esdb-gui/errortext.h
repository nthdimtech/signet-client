#ifndef ERRORTEXT_H
#define ERRORTEXT_H

#include <QLabel>

class errorText : public QLabel
{
	Q_OBJECT
public:
	errorText(const QString &name);
	~errorText();
};

#endif
