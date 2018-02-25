#include "newgeneric.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

#include "databasefield.h"
#include "buttonwaitdialog.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

#include "generic.h"
#include "generictypedesc.h"

#include "signetapplication.h"
#include "genericfieldseditor.h"
#include "typedesceditor.h"

NewGeneric::NewGeneric(int id, genericTypeDesc *typeDesc, const QString &name, QWidget *parent) :
	QDialog(parent),
	m_typeDesc(typeDesc),
	m_id(id),
	m_signetdevCmdToken(-1)
{
	setWindowModality(Qt::WindowModal);
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	QBoxLayout *nameLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	m_nameField = new QLineEdit(name);
	nameLayout->addWidget(new QLabel("Name"));
	nameLayout->addWidget(m_nameField);

	QPushButton *createButton = new QPushButton("Create");
	connect(createButton, SIGNAL(pressed()), this, SLOT(createButtonPressed()));
	createButton->setDefault(true);

	QPushButton *closeButton = new QPushButton("Close");
	connect(closeButton, SIGNAL(pressed()), this, SLOT(close()));

	QHBoxLayout *buttons = new QHBoxLayout();
	buttons->addWidget(createButton);
	buttons->addWidget(closeButton);

	bool isUserType = !typeDesc->name.compare(QString("User type"), Qt::CaseInsensitive);

	if (isUserType) {
		m_genericFieldsEditor = new TypeDescEditor(m_genericFields,
							m_typeDesc->fields);
	} else {
		m_genericFieldsEditor = new GenericFieldsEditor(m_genericFields,
								m_typeDesc->fields);
	}

	setWindowTitle("New " + m_typeDesc->name);
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
	layout->setAlignment(Qt::AlignTop);
	layout->addLayout(nameLayout);
	layout->addWidget(m_genericFieldsEditor);
	layout->addLayout(buttons);
	setLayout(layout);
}

void NewGeneric::createButtonPressed()
{
	QString action = "add " + this->m_typeDesc->name + " \"" + m_nameField->text() + "\"";
	m_buttonWaitDialog = new ButtonWaitDialog("Add " + m_typeDesc->name, action, this);
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(addEntryFinished(int)));
	m_buttonWaitDialog->show();
	block blk;

	m_genericFieldsEditor->saveFields();
	generic *g = new generic(m_id);
	g->fields = m_genericFields;
	g->name = m_nameField->text();
	g->typeName = m_typeDesc->name;
	m_entry = g;
	g->toBlock(&blk);
	::signetdev_update_uid(NULL, &m_signetdevCmdToken,
				   g->id,
				   blk.data.size(),
				   (const u8 *)blk.data.data(),
				   (const u8 *)blk.mask.data());
}

void NewGeneric::signetdevCmdResp(signetdevCmdRespInfo info)
{
	int code = info.resp_code;

	if (m_signetdevCmdToken != info.token) {
		return;
	}
	m_signetdevCmdToken = -1;

	if (m_buttonWaitDialog) {
		m_buttonWaitDialog->done(QMessageBox::Ok);
	}

	switch (code) {
	case OKAY: {
		switch (info.cmd) {
		case SIGNETDEV_CMD_UPDATE_UID:
			emit entryCreated(m_entry);
			close();
			break;
		}
	}
	break;
	case BUTTON_PRESS_TIMEOUT:
	case BUTTON_PRESS_CANCELED:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		break;
	default: {
		emit abort();
		close();
	}
	break;
	}
}

void NewGeneric::addEntryFinished(int code)
{
	if (m_buttonWaitDialog)
		m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
}
