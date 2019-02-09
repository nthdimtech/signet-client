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
#include "signetdev/host/signetdev.h"
};

DatabaseField::DatabaseField(const QString &name, int width, QList<QWidget *> &widgets, QWidget *parent) : QWidget(parent),
	m_buttonWait(NULL),
	m_name(name),
	m_signetdevCmdToken(-1)
{
	init(width, widgets);
}


DatabaseField::DatabaseField(const QString &name, int width, QWidget *middle, QWidget *parent) : QWidget(parent),
	m_buttonWait(NULL),
	m_name(name),
	m_signetdevCmdToken(-1)
{
	QList<QWidget *> widgets;
	if (middle)
		widgets.append(middle);
	init(width, widgets);
}

void DatabaseField::init(int width, QList<QWidget *> &widgets)
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

	for (auto w : widgets) {
		layout->addWidget(w);
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

void DatabaseField::retryTypeData()
{
	if (m_buttonWait) {
		m_buttonWait->resetTimeout();
	}
	::signetdev_button_wait(NULL, &m_signetdevCmdToken);
}

void DatabaseField::signetdevCmdResp(signetdevCmdRespInfo info)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;

	int code = info.resp_code;

	switch (code) {
	case OKAY:
		switch (info.cmd) {
		case SIGNETDEV_CMD_BUTTON_WAIT: {
			if (QApplication::focusWindow()) {
				QMessageBox *box = SignetApplication::messageBoxError(
						       QMessageBox::Warning,
						       "Signet",
						       "A destination text area must be selected for typing to start\n\n"
						       "Click OK and try again.", m_buttonWait ? (QWidget *)m_buttonWait : (QWidget *)this);
				connect(box, SIGNAL(finished(int)), this, SLOT(retryTypeData()));
				break;
			}
			if (m_buttonWait) {
				m_buttonWait->done(QMessageBox::Ok);
			}
			//Sleep(10);
			::signetdev_type_w(NULL, &m_signetdevCmdToken,
					       (u16 *)m_keysToType.data(), m_keysToType.length());
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
		if (m_buttonWait)
			m_buttonWait->done(QMessageBox::Ok);
		break;
	default:
		if (m_buttonWait)
			m_buttonWait->done(QMessageBox::Ok);
		abort();
		return;
	}
}

void DatabaseField::typeFieldUi()
{
	QString keys = m_fieldEdit->text();
	m_keysToType.clear();
	for (auto key : keys) {
		m_keysToType.append(key.unicode());
	}
	if (!::signetdev_can_type_w(m_keysToType.data(), m_keysToType.length())) {
		QMessageBox *msg = new QMessageBox(QMessageBox::Warning,
					"Cannot type data",
					"Signet cannot type this data. It contains characters not present in your keyboard layout.",
					QMessageBox::NoButton, this);
		QPushButton *copyData = msg->addButton("Copy data", QMessageBox::AcceptRole);
		msg->addButton("Cancel", QMessageBox::RejectRole);
		msg->setWindowModality(Qt::WindowModal);
		msg->exec();
		QAbstractButton *button = msg->clickedButton();
		if (button == copyData) {
			copyFieldUi();
		}
		msg->deleteLater();
		return;
	}
	m_buttonWait = new ButtonWaitDialog("Type " + m_name, "type " + m_name, (QWidget *)this->parent());
	connect(m_buttonWait, SIGNAL(finished(int)), this, SLOT(typeFieldFinished(int)));
	m_buttonWait->show();
	::signetdev_button_wait(NULL, &m_signetdevCmdToken);
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
