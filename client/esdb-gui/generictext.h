#ifndef GENERICTEXT_H
#define GENERICTEXT_H

#include <QLabel>

class genericText : public QLabel
{
	Q_OBJECT
public:
	genericText(const QString &name);
	~genericText();
};

#endif
