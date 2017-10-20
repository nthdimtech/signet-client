#include "opengeneric.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QCheckBox>
#include <QDesktopServices>

#include "databasefield.h"
#include "account.h"
#include "buttonwaitdialog.h"
#include "signetapplication.h"
#include "generic.h"
#include "generictypedesc.h"
#include "genericfieldseditor.h"
#include "typedesceditor.h"

extern "C" {
#include "signetdev.h"
}

OpenGeneric::OpenGeneric(generic *generic, genericTypeDesc *typeDesc, QWidget *parent) :
	QDialog(parent),
	m_generic(generic),
	m_typeDesc(typeDesc),
	m_buttonWaitDialog(NULL),
	m_fields(generic->fields),
	m_signetdevCmdToken(-1),
	m_settingFields(false)
{
	setWindowModality(Qt::WindowModal);
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	setWindowTitle(generic->name);
	m_genericNameEdit = new QLineEdit();

	QBoxLayout *nameLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	nameLayout->addWidget(new QLabel("Name"));
	nameLayout->addWidget(m_genericNameEdit);

	connect(m_genericNameEdit, SIGNAL(textEdited(QString)), this, SLOT(edited()));
	bool isUserType = !typeDesc->name.compare(QString("User type"), Qt::CaseInsensitive);
	if (isUserType) {
		m_genericFieldsEditor = new TypeDescEditor(m_fields,
							m_typeDesc->fields);
	} else {
		m_genericFieldsEditor = new GenericFieldsEditor(m_fields,
								m_typeDesc->fields);
	}
	connect(m_genericFieldsEditor, SIGNAL(edited()), this, SLOT(edited()));

	m_settingFields = true;
	m_genericNameEdit->setText(m_generic->name);
	m_genericFieldsEditor->loadFields();
	m_settingFields = false;

	m_undoChangesButton = new QPushButton("Undo");

	m_saveButton = new QPushButton("Save");
	m_saveButton->setDisabled(true);
	m_saveButton->setDefault(true);

	QPushButton *closeButton = new QPushButton("Close");
	m_undoChangesButton->setDisabled(true);
	connect(m_undoChangesButton, SIGNAL(pressed()), this, SLOT(undoChangesUI()));

	QHBoxLayout *buttons = new QHBoxLayout();

	buttons->addWidget(m_saveButton);
	buttons->addWidget(m_undoChangesButton);
	buttons->addWidget(closeButton);

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setAlignment(Qt::AlignTop);
	mainLayout->addLayout(nameLayout);
	mainLayout->addWidget(m_genericFieldsEditor);
	mainLayout->addLayout(buttons);
	setLayout(mainLayout);

	connect(m_saveButton, SIGNAL(pressed(void)), this, SLOT(savePressed(void)));
	connect(closeButton, SIGNAL(pressed(void)), this, SLOT(closePressed(void)));
}

void OpenGeneric::signetdevCmdResp(signetdevCmdRespInfo info)
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
		case SIGNETDEV_CMD_WRITE_ID:
			m_generic->name = m_genericNameEdit->text();
			m_generic->fields = m_fields;
			emit accountChanged(m_generic->id);
			m_saveButton->setDisabled(true);
			m_undoChangesButton->setDisabled(true);
			break;
		}
	}
	break;
	case BUTTON_PRESS_TIMEOUT:
	case BUTTON_PRESS_CANCELED:
		m_saveButton->setDisabled(false);
		m_undoChangesButton->setDisabled(false);
		break;
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		close();
		break;
	default: {
		emit abort();
	}
	break;
	}
}

OpenGeneric::~OpenGeneric()
{
}

void OpenGeneric::undoChangesUI()
{
	m_settingFields = true;
	m_genericNameEdit->setText(m_generic->name);
	m_genericFieldsEditor->loadFields();
	m_settingFields = false;
	m_saveButton->setDisabled(true);
	m_undoChangesButton->setDisabled(true);
}

void OpenGeneric::edited()
{
	if (!m_settingFields) {
		m_saveButton->setDisabled(false);
		m_undoChangesButton->setDisabled(false);
	}
}

void OpenGeneric::closePressed()
{
	close();
}

void OpenGeneric::savePressed()
{
	m_buttonWaitDialog = new ButtonWaitDialog( "Open " + m_typeDesc->name,
		QString("save changes to " + m_typeDesc->name.toLower() + " \"") + m_genericNameEdit->text() + QString("\""),
		this);
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(saveGenericFinished(int)));
	m_buttonWaitDialog->show();

	m_genericFieldsEditor->saveFields();
	block blk;
	generic g(m_generic->id);
	g.name = m_genericNameEdit->text();
	g.typeName = m_typeDesc->name;
	g.fields = m_fields;
	g.toBlock(&blk);
	::signetdev_write_id_async(NULL, &m_signetdevCmdToken,
				   m_generic->id,
				   blk.data.size(),
				   (const u8 *)blk.data.data(),
				   (const u8 *)blk.mask.data());
}

void OpenGeneric::saveGenericFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
}
