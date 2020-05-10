#include "typedescedit.h"

#include <QComboBox>

typeDescEdit::typeDescEdit(const QString &name, bool canRemove, bool secretField, QWidget *parent) :
    genericFieldEdit(name, secretField, parent)
{
	m_enableHideCheckbox = false;
	m_typeEditCombo = new QComboBox();
	m_typeEditCombo->setEnabled(!SignetApplication::get()->isDeviceEmulated());
	m_typeEditCombo->addItem("Text", "Text");
	m_typeEditCombo->addItem("Text (Secret)", ".Text");
	m_typeEditCombo->addItem("Text block", "Text block");
	m_typeEditCombo->addItem("Integer", "Integer");
	connect(m_typeEditCombo, SIGNAL(currentIndexChanged(int)),
        this, SIGNAL(edited()));
	createWidget(canRemove, m_typeEditCombo, false);
}

QString typeDescEdit::toString() const
{
	return m_typeEditCombo->currentData().toString();
}

void typeDescEdit::fromString(const QString &s)
{
	int i;
	for (i = 0; i < m_typeEditCombo->count(); i++) {
		if (!m_typeEditCombo->itemData(i).toString().compare(s, Qt::CaseInsensitive)) {
			m_typeEditCombo->setCurrentIndex(i);
			break;
		}
	}
	if (i == m_typeEditCombo->count()) {
		m_typeEditCombo->setCurrentIndex(0);
	}
}
