#include "typedescedit.h"

#include <QComboBox>

typeDescEdit::typeDescEdit(const QString &name, bool canRemove, QWidget *parent) :
        genericFieldEdit(name, parent)
{
	m_typeEditCombo = new QComboBox();
	m_typeEditCombo->setEnabled(!SignetApplication::get()->isDeviceEmulated());
	m_typeEditCombo->addItem("Text");
	m_typeEditCombo->addItem("Text block");
	m_typeEditCombo->addItem("Integer");
	connect(m_typeEditCombo, SIGNAL(currentIndexChanged(int)),
	        this, SIGNAL(edited()));
	createWidget(canRemove, m_typeEditCombo, false);
}

QString typeDescEdit::toString() const
{
	return m_typeEditCombo->currentText();
}

void typeDescEdit::fromString(const QString &s)
{
	int i;
	for (i = 0; i < m_typeEditCombo->count(); i++) {
		if (!m_typeEditCombo->itemText(i).compare(s, Qt::CaseInsensitive)) {
			m_typeEditCombo->setCurrentIndex(i);
			break;
		}
	}
	if (i == m_typeEditCombo->count()) {
		m_typeEditCombo->setCurrentIndex(0);
	}
}
