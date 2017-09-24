#include "textblockfieldedit.h"

#include <QTextEdit>
#include <QString>

textBlockFieldEdit::textBlockFieldEdit(const QString &name, bool canRemove) :
	genericFieldEdit(name)
{
	m_textEdit = new QTextEdit();
	connect(m_textEdit, SIGNAL(textChanged()),
		this, SIGNAL(edited()));
	createTallWidget(3, canRemove, m_textEdit);
}

QString textBlockFieldEdit::toString() const
{
	return m_textEdit->toPlainText();
}

void textBlockFieldEdit::fromString(const QString &s)
{
	m_textEdit->setPlainText(s);
}
