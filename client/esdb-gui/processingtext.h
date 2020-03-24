#ifndef PROCESSINGTEXT_H
#define PROCESSINGTEXT_H

#include <QLabel>

class processingText : public QLabel
{
	Q_OBJECT
public:
	processingText(const QString &name);
	~processingText();
};

#endif
