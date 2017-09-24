#include "genericfieldedit.h"

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QClipboard>
#include <QApplication>
#include <QGridLayout>

genericFieldEdit::genericFieldEdit(const QString &name) :
	m_name(name),
	m_widget(NULL)
{

}
void genericFieldEdit::createWidget(bool canRemove, QWidget *editWidget, bool outputEnable)
{
	m_editWidget = editWidget;
	m_widget = new QWidget();
	m_widget->setLayout(new QHBoxLayout());
	if (outputEnable) {
		m_copyButton = new QPushButton(QIcon(":/images/clipboard.png"),"");
		m_typeButton = new QPushButton(QIcon(":/images/keyboard.png"),"");
	}
	if (canRemove) {
		m_deleteButton = new QPushButton(QIcon(":/images/delete.png"),"");
	}
	m_editWidget->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));
	m_widget->layout()->setContentsMargins(0,0,0,0);
	m_widget->layout()->addWidget(new QLabel(m_name));
	m_widget->layout()->addWidget(m_editWidget);
	if (outputEnable) {
		m_widget->layout()->addWidget(m_copyButton);
		m_widget->layout()->addWidget(m_typeButton);
	}
	if (canRemove) {
		m_widget->layout()->addWidget(m_deleteButton);
	}
	if (outputEnable) {
		connect(m_copyButton, SIGNAL(clicked(bool)), this, SLOT(copyPressed()));
		connect(m_typeButton, SIGNAL(clicked(bool)), this, SLOT(typePressed()));
	}
	if (canRemove) {
		connect(m_deleteButton, SIGNAL(clicked(bool)), this, SLOT(deletePressed()));
	}
}

void genericFieldEdit::createTallWidget(int rows, bool canRemove, QWidget *editWidget)
{
	m_editWidget = editWidget;
	m_widget = new QWidget();
	QGridLayout *grid = new QGridLayout();
	m_widget->setLayout(grid);
	m_copyButton = new QPushButton(QIcon(":/images/clipboard.png"),"");
	m_typeButton = new QPushButton(QIcon(":/images/keyboard.png"),"");
	if (canRemove) {
		m_deleteButton = new QPushButton(QIcon(":/images/delete.png"),"");
	}
	m_copyButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_typeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	if (canRemove) {
		m_deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	}
	m_editWidget->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::QSizePolicy::Ignored));
	m_editWidget->setContentsMargins(0,0,0,0);
	grid->setContentsMargins(0,0,0,0);
	grid->setColumnStretch(1,1);
	QLabel *label = new QLabel(m_name);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	grid->addWidget(label, 0, 0, 1, 1, Qt::AlignTop);
	grid->addWidget(m_editWidget, 0, 1, rows, 1, Qt::AlignCenter);
	grid->addWidget(m_copyButton, 0, 2, 1, 1, Qt::AlignLeft);
	grid->addWidget(m_typeButton, 1, 2, 1, 1, Qt::AlignLeft);
	if (canRemove) {
		grid->addWidget(m_deleteButton, 2, 2, 1, 1, Qt::AlignLeft);
	}
	connect(m_copyButton, SIGNAL(clicked(bool)), this, SLOT(copyPressed()));
	connect(m_typeButton, SIGNAL(clicked(bool)), this, SLOT(typePressed()));
	if (canRemove) {
		connect(m_deleteButton, SIGNAL(clicked(bool)), this, SLOT(deletePressed()));
	}
}

void genericFieldEdit::typePressed()
{
	emit type(toString());
}

void genericFieldEdit::copyPressed()
{
	QClipboard *cb = QApplication::clipboard();
	QString text = toString();
	cb->setText(text);
}

void genericFieldEdit::deletePressed()
{
	emit remove(m_name);
}
