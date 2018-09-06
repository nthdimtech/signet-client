#include "opengeneric.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QCheckBox>
#include <QDesktopServices>
#include <QCloseEvent>

#include "databasefield.h"
#include "account.h"
#include "buttonwaitdialog.h"
#include "signetapplication.h"
#include "generic.h"
#include "generictypedesc.h"
#include "genericfieldseditor.h"
#include "typedesceditor.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

OpenGeneric::OpenGeneric(generic *generic, genericTypeDesc *typeDesc, QWidget *parent) :
	QDialog(parent),
	m_generic(generic),
	m_typeDesc(typeDesc),
	m_buttonWaitDialog(NULL),
	m_fields(generic->fields),
	m_signetdevCmdToken(-1),
	m_settingFields(false),
	m_changesMade(false),
	m_closeOnSave(false)
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

	m_saveButton = new QPushButton("&Save");
	m_saveButton->setDisabled(true);
	m_saveButton->setDefault(true);

	QPushButton *closeButton = new QPushButton("Close");
	m_undoChangesButton->setDisabled(true);
	connect(m_undoChangesButton, SIGNAL(pressed()), this, SLOT(undoChangesUI()));

	QHBoxLayout *buttons = new QHBoxLayout();

	buttons->addWidget(m_saveButton);
	buttons->addWidget(m_undoChangesButton);
	buttons->addWidget(closeButton);

	m_dataOversized = new QLabel("Warning: this entry is too large. You must delete something to save it");
	m_dataOversized->setStyleSheet("QLabel { color : red; }");
	m_dataOversized->hide();

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setAlignment(Qt::AlignTop);
	mainLayout->addLayout(nameLayout);
	mainLayout->addWidget(m_genericFieldsEditor);
	mainLayout->addWidget(m_dataOversized);
	mainLayout->addLayout(buttons);
	setLayout(mainLayout);

	connect(m_saveButton, SIGNAL(pressed(void)), this, SLOT(savePressed(void)));
	connect(closeButton, SIGNAL(pressed(void)), this, SLOT(close(void)));
}

void OpenGeneric::oversizedDialog()
{
	m_isOversized = true;
	m_saveButton->setDisabled(true);
	m_dataOversized->show();
	auto mb = SignetApplication::messageBoxWarn("Entry too large", "This entry is too large. You must delete something to save it", this);
	mb->exec();
	mb->deleteLater();
}

bool OpenGeneric::toBlock(block &blk)
{
	m_genericFieldsEditor->saveFields();
	generic g(m_generic->id);
	g.name = m_genericNameEdit->text();
	g.typeName = m_typeDesc->name;
	g.fields = m_fields;
	g.toBlock(&blk);
	return blk.data.size() <= MAX_ENT_DATA_SIZE;
}

void OpenGeneric::closeEvent(QCloseEvent *event)
{
	if (m_changesMade) {
		QMessageBox *box = new QMessageBox(QMessageBox::Question, windowTitle(),
					       "You have made changes. Do you want to save them",
					       QMessageBox::Yes |
					       QMessageBox::No,
					       this);
		int rc = box->exec();
		box->deleteLater();
		if (rc == QMessageBox::Yes) {
			m_closeOnSave = true;
			event->ignore();
			savePressed();
			return;
		}
	}
	event->accept();
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
		case SIGNETDEV_CMD_UPDATE_UID:
			m_generic->name = m_genericNameEdit->text();
			m_generic->fields = m_fields;
			emit accountChanged(m_generic->id);
			m_saveButton->setDisabled(true);
			m_undoChangesButton->setDisabled(true);
			m_changesMade = false;
			if (m_closeOnSave) {
				close();
			}
			break;
		}
	}
	break;
	case NOT_ENOUGH_SPACE:
		oversizedDialog();
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
	m_changesMade = false;
}

void OpenGeneric::edited()
{
	if (!m_settingFields) {
		m_saveButton->setDisabled(false);
		m_undoChangesButton->setDisabled(false);
		m_changesMade = true;
		block blk;
		if (!toBlock(blk)) {
			m_isOversized = true;
			m_saveButton->setDisabled(true);
			m_dataOversized->show();
		} else {
			m_isOversized = false;
			m_saveButton->setDisabled(false);
			m_dataOversized->hide();
		}
	}
}

void OpenGeneric::savePressed()
{
	m_genericFieldsEditor->saveFields();
	block blk;
	if (toBlock(blk)) {
		m_buttonWaitDialog = new ButtonWaitDialog( "Save " + m_typeDesc->name,
			QString("save changes to " + m_typeDesc->name.toLower() + " \"") + m_genericNameEdit->text() + QString("\""),
			this);
		connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(saveGenericFinished(int)));
		m_buttonWaitDialog->show();
		::signetdev_update_uid(NULL, &m_signetdevCmdToken,
					   m_generic->id,
					   blk.data.size(),
					   (const u8 *)blk.data.data(),
					   (const u8 *)blk.mask.data());
	} else {
		oversizedDialog();
	}
}

void OpenGeneric::saveGenericFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
}
