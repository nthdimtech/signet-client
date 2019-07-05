#include "editentrydialog.h"
#include "buttonwaitdialog.h"
#include "esdb.h"

#include "signetapplication.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCloseEvent>

EditEntryDialog::EditEntryDialog(QString typeName, int id, QWidget *parent) :
	QDialog(parent),
    m_typeName(typeName),
	m_signetdevCmdToken(-1),
	m_isOversized(false),
	m_dataOversized(nullptr),
	m_buttonWaitDialog(nullptr),
	m_undoChangesButton(nullptr),
	m_changesMade(false),
	m_closeOnSave(false),
	m_id(id),
	m_settingFields(false),
    m_entry(nullptr)
{
	m_isNew = true;
	setWindowModality(Qt::WindowModal);
	setWindowTitle("New " + typeName);
}


EditEntryDialog::EditEntryDialog(QString typeName, esdbEntry *ent, QWidget *parent) :
	QDialog(parent),
	m_typeName(typeName),
	m_signetdevCmdToken(-1),
	m_isOversized(false),
	m_dataOversized(nullptr),
	m_buttonWaitDialog(nullptr),
	m_undoChangesButton(nullptr),
	m_submitButton(nullptr),
	m_changesMade(false),
	m_closeOnSave(false),
	m_id(ent->id),
	m_settingFields(false),
	m_entry(ent)
{
	setWindowModality(Qt::WindowModal);
	setWindowTitle("Edit " + ent->getTitle());
	m_isNew = false;
}

void EditEntryDialog::setupBase()
{
	setWindowModality(Qt::WindowModal);
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	m_submitButton = new QPushButton(m_isNew ? "&Create" : "&Save");
	connect(m_submitButton, SIGNAL(pressed()), this, SLOT(submitButtonPressed()));
	m_submitButton->setDefault(true);
	m_submitButton->setDisabled(true);

	if (!m_isNew) {
		m_undoChangesButton = new QPushButton("Undo");
		m_undoChangesButton->setDisabled(true);
		connect(m_undoChangesButton, SIGNAL(pressed()), SLOT(undoButtonPressed()));
	}

	QPushButton *closeButton = new QPushButton("Close");
	connect(closeButton, SIGNAL(pressed()), this, SLOT(accept()));

	m_buttons = new QBoxLayout(QBoxLayout::LeftToRight);
	m_buttons->addWidget(m_submitButton);
	if (!m_isNew) {
		m_buttons->addWidget(m_undoChangesButton);
	}
	m_buttons->addWidget(closeButton);

	m_dataOversized = new QLabel("Warning: this entry is too large. You must delete something to save it");
	m_dataOversized->setStyleSheet("QLabel { color : red; }");
	m_dataOversized->hide();
}

void EditEntryDialog::setup(QLayout *layout)
{
	setupBase();
	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setAlignment(Qt::AlignTop);
	mainLayout->addLayout(layout);
	mainLayout->addWidget(m_dataOversized);
	mainLayout->addLayout(m_buttons);
	setLayout(mainLayout);
}

void EditEntryDialog::setup(QWidget *widget)
{
	setupBase();
	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setAlignment(Qt::AlignTop);
	mainLayout->addWidget(widget);
	mainLayout->addWidget(m_dataOversized);
	mainLayout->addLayout(m_buttons);
	setLayout(mainLayout);
}

void EditEntryDialog::oversizedDialog()
{
	m_isOversized = true;
	m_submitButton->setDisabled(true);
	m_dataOversized->show();
	auto mb = SignetApplication::messageBoxWarn("Entry too large", "This entry is too large. You must delete something to save it", this);
}

void EditEntryDialog::submitButtonPressed()
{
	QString title = (m_isNew ? "Add " : "Save ") + m_typeName;
	QString action = (m_isNew ? "save " : "save changes so") + m_typeName.toLower() + " \"" + entryName() + "\"";

	block blk;
	auto tmp = createEntry(m_id);
	applyChanges(tmp);
	tmp->toBlock(&blk);

	if (m_isNew) {
		m_entry = tmp;
	} else {
		delete tmp;
	}

	if (blk.data.size() > MAX_ENT_DATA_SIZE) {
		if (m_isNew) {
			delete m_entry;
			m_entry = nullptr;
		}
		oversizedDialog();
		return;
	}

	m_buttonWaitDialog = new ButtonWaitDialog(title, action, this);
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(submitEntryFinished(int)));
	m_buttonWaitDialog->show();

	::signetdev_update_uid(nullptr, &m_signetdevCmdToken,
                   m_id,
                   blk.data.size(),
                   (const u8 *)blk.data.data(),
			       (const u8 *)blk.mask.data());
}

void EditEntryDialog::undoButtonPressed()
{
	m_settingFields = true;
	undoChanges();
	m_settingFields = false;
	entryNameEdited();
	m_changesMade = false;
	m_undoChangesButton->setDisabled(true);
	m_submitButton->setDisabled(true);
}

void EditEntryDialog::submitEntryFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = nullptr;
}


void EditEntryDialog::signetdevCmdResp(signetdevCmdRespInfo info)
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
			if (m_isNew) {
				emit entryCreated(m_entry);
				accept();
			} else {
				applyChanges(m_entry);
				emit entryChanged(m_id);
				m_submitButton->setDisabled(true);
				m_undoChangesButton->setDisabled(true);
				m_changesMade = false;
				if (m_closeOnSave) {
					accept();
				}
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
		m_submitButton->setDisabled(false);
		if (!m_isNew) {
			m_undoChangesButton->setDisabled(false);
		}
		break;
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		reject();
		break;
	default: {
		emit abort();
	}
	break;
	}
}

void EditEntryDialog::edited()
{
	if (!m_settingFields) {
		m_submitButton->setDisabled(false);
		if (!m_isNew) {
			m_undoChangesButton->setDisabled(false);
		}
		m_changesMade = true;

		block blk;
		auto tmp = createEntry(m_id);
		applyChanges(tmp);
		tmp->toBlock(&blk);
		delete tmp;
		if (blk.data.size() > MAX_ENT_DATA_SIZE) {
			m_isOversized = true;
			m_submitButton->setDisabled(true);
			m_dataOversized->show();
		} else {
			m_isOversized = false;
			m_submitButton->setDisabled(false);
			m_dataOversized->hide();
		}
	}
}

void EditEntryDialog::entryNameEdited()
{
	if (!m_isNew) {
		setWindowTitle("Edit " + entryName());
	}
}

void EditEntryDialog::closeEvent(QCloseEvent *event)
{
	if (m_changesMade && !m_isNew) {
		QMessageBox *saveOnClose = new QMessageBox(QMessageBox::Question, windowTitle(),
                "You have made changes. Do you want to save them",
                QMessageBox::Yes |
                QMessageBox::No,
                this);
		connect(saveOnClose, SIGNAL(finished(int)), this, SLOT(saveOnCloseDialogFinished(int)));
		saveOnClose->setWindowModality(Qt::WindowModal);
		saveOnClose->setAttribute(Qt::WA_DeleteOnClose);
		saveOnClose->show();
		event->ignore();
		return;
	}
	event->accept();
}

void EditEntryDialog::saveOnCloseDialogFinished(int rc)
{
	if (rc == QMessageBox::Yes) {
		submitButtonPressed();
	} else {
		m_changesMade = false;
		done(0);
	}
}
