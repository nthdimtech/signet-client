#include "cleartextpasswordselector.h"
#include "cleartextpasswordeditor.h"
#include "signetapplication.h"
#include "buttonwaitdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QRadioButton>
#include <QPushButton>
#include <QLabel>

extern "C" {
#include "signetdev/host/signetdev.h"
}

#include "signetapplication.h"
#include "generictext.h"

cleartextPasswordSelector::cleartextPasswordSelector(QVector<int> formats, QStringList names, QWidget *parent) :
	QDialog(parent),
	m_formats(formats),
	m_buttonWaitDialog(NULL)
{
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevReadCleartextPassword(signetdevCmdRespInfo,cleartext_pass)),
		this, SLOT(signetdevReadCleartextPassword(signetdevCmdRespInfo,cleartext_pass)));
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));
	setWindowModality(Qt::WindowModal);
	QVBoxLayout *l = new QVBoxLayout();
	QHBoxLayout *buttons = new QHBoxLayout();
	m_openButton = new QPushButton("Open");
	QPushButton *closeButton = new QPushButton("Close");
	m_deleteButton = new QPushButton("Delete");

	if (formats.size() == names.size()) {
		for (int i = 0; i < formats.size(); i++) {
			QString n;
			if (formats.at(i) != 1) {
				n = "<Unused>";
			} else {
				n = names.at(i);
			}
			QHBoxLayout *slot = new QHBoxLayout();
			QRadioButton *b = new QRadioButton(n);
			connect(b, SIGNAL(pressed()), this, SLOT(selectionPressed()));
			slot->addWidget(new genericText("Slot #" + QString::number(i+1)));
			slot->addWidget(b);

			m_slotButtons.append(b);
			l->addLayout(slot);
		}
	}
	buttons->addWidget(m_openButton);
	buttons->addWidget(m_deleteButton);
	buttons->addWidget(closeButton);
	l->addLayout(buttons);
	setLayout(l);

	m_openButton->setDisabled(true);
	m_deleteButton->setDisabled(true);

	connect(closeButton, SIGNAL(pressed()), this, SLOT(close()));
	connect(m_deleteButton, SIGNAL(pressed()), this, SLOT(deletePressed()));
	connect(m_openButton, SIGNAL(pressed()), this, SLOT(openPressed()));
}

void cleartextPasswordSelector::deletePressed()
{
	int i = 0;
	for (auto b : m_slotButtons) {
		if (b->isChecked()) {
			m_index = i;
			m_buttonWaitDialog = new ButtonWaitDialog("Delete password slot", "delete password slot", this, false);
			connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(buttonWaitFinished(int)));
			m_buttonWaitDialog->show();
			struct cleartext_pass p;
			memset(&p, 0, sizeof(p));
			p.format = 0xff;
			m_formats.replace(m_index, 0xff);
			::signetdev_write_cleartext_password(NULL, &m_signetdevCmdToken, m_index, &p);
			break;
		}
		i++;
	}
}

void cleartextPasswordSelector::selectionPressed()
{
	m_openButton->setDisabled(false);
	int i = 0;
	for (auto b : m_slotButtons) {
		if (b == QObject::sender()) {
			m_deleteButton->setDisabled(m_formats.at(i) != 1);
		}
		i++;
	}
}

void cleartextPasswordSelector::buttonWaitFinished(int result)
{
	m_buttonWaitDialog->deleteLater();
	if (result != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog = NULL;
}


void cleartextPasswordSelector::signetdevCmdResp(signetdevCmdRespInfo info)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	if (m_buttonWaitDialog) {
		m_buttonWaitDialog->done(0);
	}
	if (info.resp_code == OKAY) {
		m_slotButtons.at(m_index)->setText("<Unused>");
		if (m_slotButtons.at(m_index)->isChecked()) {
			m_deleteButton->setDisabled(true);
		}
	} else {
		SignetApplication::messageBoxError(QMessageBox::Critical,
                           "Save password slot",
                           "Failed to delete password slot",
                           this);
	}
}

void cleartextPasswordSelector::signetdevReadCleartextPassword(signetdevCmdRespInfo info, cleartext_pass pass)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;
	if (m_buttonWaitDialog)
		m_buttonWaitDialog->done(QMessageBox::Ok);
	if (info.resp_code == OKAY) {
		cleartextPasswordEditor *e = new cleartextPasswordEditor(m_index, &pass, this);
		e->setWindowTitle("Password slot " + QString::number(m_index + 1));
		e->setMinimumWidth(300);
		e->exec();
		e->deleteLater();
		m_formats.replace(m_index, pass.format);
		m_slotButtons.at(m_index)->setText(QString::fromUtf8(pass.name_utf8));
		if (pass.format != 1) {
			m_slotButtons.at(m_index)->setText("<Unused>");
		} else {
			m_slotButtons.at(m_index)->setText(QString::fromUtf8(pass.name_utf8));
		}
		if (m_slotButtons.at(m_index)->isChecked()) {
			m_deleteButton->setDisabled(pass.format != 1);
		}
	} else {
		QMessageBox *box = SignetApplication::messageBoxError(QMessageBox::Critical,
                   QString("Read password slot"),
                   QString("Failed to read password slot ") +
                   QString::number(m_index),
                   this);
		box->exec();
	}
}

void cleartextPasswordSelector::openPressed()
{
	int i = 0;
	for (auto b : m_slotButtons) {
		if (b->isChecked()) {
			m_index = i;
			m_buttonWaitDialog = new ButtonWaitDialog("Open password slot", "open password slot", this, false);
			connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(buttonWaitFinished(int)));
			m_buttonWaitDialog->show();
			::signetdev_read_cleartext_password(NULL, &m_signetdevCmdToken, m_index);
			break;
		}
		i++;
	}
}
