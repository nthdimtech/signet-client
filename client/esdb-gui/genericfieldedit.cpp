#include "genericfieldedit.h"
#include "buttonwaitdialog.h"
#include "signetapplication.h"

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QClipboard>
#include <QApplication>
#include <QGridLayout>
#include <QCheckBox>

extern "C" {
#include "signetdev/host/signetdev.h"
}

genericFieldEdit::genericFieldEdit(const QString &name) :
	m_name(name),
	m_widget(nullptr),
	m_signetdevCmdToken(-1)
{
	SignetApplication *app = SignetApplication::get();
	QObject::connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
			 this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));
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
	m_widget->layout()->addWidget(new QLabel(displayName()));
	m_widget->layout()->addWidget(m_editWidget);

	if (isSecretField()) {
		m_secretCheckbox = new QCheckBox("Hide");
		m_secretCheckbox->setCheckState(Qt::Checked);
		hideContent();
		m_widget->layout()->addWidget(m_secretCheckbox);
		connect(m_secretCheckbox, SIGNAL(stateChanged(int)), SLOT(secretCheckStateChanged(int)));
	}
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
	QString keys = toString();
	m_keysToType.clear();
	for (auto key : keys) {
		m_keysToType.append(key.unicode());
	}
	if (!::signetdev_can_type_w(m_keysToType.data(), m_keysToType.length())) {
		QMessageBox *msg = new QMessageBox(QMessageBox::Warning,
					"Cannot type data",
					"Signet cannot type this data. It contains characters not present in your keyboard layout.",
					QMessageBox::NoButton, m_editWidget);
		QPushButton *copyData = msg->addButton("Copy data", QMessageBox::AcceptRole);
		msg->addButton("Cancel", QMessageBox::RejectRole);
		msg->setWindowModality(Qt::WindowModal);
		msg->exec();
		QAbstractButton *button = msg->clickedButton();
		if (button == copyData) {
			copyPressed();
		}
		msg->deleteLater();
		return;
	}
	m_buttonWait = new ButtonWaitDialog("Type " + m_name, "type " + m_name, (QWidget *)this->parent());
	connect(m_buttonWait, SIGNAL(finished(int)), this, SLOT(typeFieldFinished(int)));
	m_buttonWait->show();
	::signetdev_button_wait(NULL, &m_signetdevCmdToken);
}

void genericFieldEdit::typeFieldFinished(int rc)
{
	if (rc != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWait->deleteLater();
	m_buttonWait = NULL;
}

void genericFieldEdit::signetdevCmdResp(signetdevCmdRespInfo info)
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
			QString keys = toString();
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
			Sleep(10);
			::signetdev_type_w(NULL, &m_signetdevCmdToken, (u16 *)m_keysToType.data(), m_keysToType.length());
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

void genericFieldEdit::secretCheckStateChanged(int state)
{
	if (state == Qt::Checked) {
		hideContent();
	} else {
		showContent();
	}
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
