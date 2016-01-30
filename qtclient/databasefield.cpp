#include "databasefield.h"

#include<QHBoxLayout>
#include<QLabel>
#include<QLineEdit>
#include<QPushButton>
#include<QApplication>
#include<QClipboard>

#include "buttonwaitdialog.h"
#include "signetapplication.h"

extern "C" {
#include "signetdev.h"
}

DatabaseField::DatabaseField(const QString &name, int width, QWidget *middle, QWidget *parent) : QWidget(parent),
	m_buttonWait(NULL),
	m_name(name),
	m_signetdevCmdToken(-1)
{
	SignetApplication *app = SignetApplication::get();

	QObject::connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
	                 this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	QPushButton *type_button = new QPushButton(QIcon(":/images/keyboard.png"),"");
	type_button->setToolTip("Type");
	type_button->setFocusPolicy(Qt::NoFocus);
	connect(type_button, SIGNAL(clicked()), this, SLOT(typeFieldUi()));

	QPushButton *copy_button = new QPushButton(QIcon(":/images/clipboard.png"),"");
	copy_button->setToolTip("Copy");
	copy_button->setFocusPolicy(Qt::NoFocus);
	connect(copy_button, SIGNAL(clicked()), this, SLOT(copyFieldUi()));

	m_fieldEdit = new QLineEdit();
	m_fieldEdit->setMinimumWidth(width);

	connect(m_fieldEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
	connect(m_fieldEdit, SIGNAL(textEdited(QString)), this, SIGNAL(textEdited(QString)));

	QHBoxLayout *layout = new QHBoxLayout();
	layout->setContentsMargins(0,0,0,0);
	QString capitalized_name = m_name;
	capitalized_name[0] = capitalized_name[0].toUpper();
	layout->addWidget(new QLabel(capitalized_name));
	layout->addWidget(m_fieldEdit);
	if (middle) {
		layout->addWidget(middle);
	}
	layout->addWidget(copy_button);
	layout->addWidget(type_button);
	setLayout(layout);
}

QString DatabaseField::text() const
{
	return m_fieldEdit->text();
}

void DatabaseField::setText(const QString &s)
{
	m_fieldEdit->setText(s);
}

void DatabaseField::signetdevCmdResp(signetdevCmdRespInfo info)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;

	int code = info.resp_code;

	if (m_buttonWait)
		m_buttonWait->done(QMessageBox::Ok);

	switch (code) {
	case OKAY:
		switch (info.cmd) {
		case SIGNETDEV_CMD_BUTTON_WAIT: {
			QByteArray keys = m_fieldEdit->text().toLatin1();
			::signetdev_type_async(NULL, &m_signetdevCmdToken,
			                       (u8 *)keys.data(), keys.length());
		}
		break;
		case SIGNETDEV_CMD_TYPE:
			break;
		}
		break;
	case BUTTON_PRESS_CANCELED:
	case BUTTON_PRESS_TIMEOUT:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		break;
	default:
		abort();
		return;
	}
}

void DatabaseField::typeFieldUi()
{
	m_buttonWait = new ButtonWaitDialog("Type " + m_name, "type " + m_name, (QWidget *)this->parent());
	connect(m_buttonWait, SIGNAL(finished(int)), this, SLOT(typeFieldFinished(int)));
	m_buttonWait->show();
	::signetdev_button_wait_async(NULL, &m_signetdevCmdToken);
}

void DatabaseField::typeFieldFinished(int rc)
{
	if (rc != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWait->deleteLater();
	m_buttonWait = NULL;
}

void DatabaseField::copyFieldUi()
{
	QClipboard *cb = QApplication::clipboard();
	cb->setText(m_fieldEdit->text());
}
