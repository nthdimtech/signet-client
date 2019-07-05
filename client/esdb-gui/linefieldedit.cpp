#include "linefieldedit.h"

#include <QLineEdit>
#include <QString>

void lineFieldEdit::showContent()
{
	m_lineEdit->setEchoMode(QLineEdit::Normal);
}

void lineFieldEdit::hideContent()
{
	m_lineEdit->setEchoMode(QLineEdit::Password);
}

lineFieldEdit::lineFieldEdit(const QString &name, bool canRemove, QWidget *parent) :
    genericFieldEdit(name, parent)
{
	m_lineEdit = new QLineEdit();
	m_lineEdit->setReadOnly(SignetApplication::get()->isDeviceEmulated());
	connect(m_lineEdit, SIGNAL(textEdited(QString)),
        this, SIGNAL(edited()));
	connect(m_lineEdit, SIGNAL(editingFinished()),
		this, SIGNAL(editingFinished()));
	createWidget(canRemove, m_lineEdit);
}

QString lineFieldEdit::toString() const
{
	return m_lineEdit->text();
}

void lineFieldEdit::fromString(const QString &s)
{
	m_lineEdit->setText(s);
}
