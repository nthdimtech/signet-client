#include "notetext.h"

noteText::noteText(const QString &name) : QLabel(name)
{
	setWordWrap(true);
}

noteText::~noteText()
{
}
