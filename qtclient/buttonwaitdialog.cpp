#include "buttonwaitdialog.h"
#include "signetapplication.h"

#include <QPixmap>
#include <QTimer>

ButtonWaitDialog::ButtonWaitDialog(QString title, QString action, QWidget *parent, bool longPress) :
	QMessageBox(QMessageBox::NoIcon, title, "", QMessageBox::Cancel, parent, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint),
	m_timeLeft(sTimeoutPeriod),
	m_action(action),
	m_longPress(longPress)
{
	setWindowModality(Qt::WindowModal);
	QPixmap pm(":/images/button_press.png");
	setIconPixmap(pm.scaledToHeight(40));
	updateText();

	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevTimerEvent(int)),
		this, SLOT(signetdevTimerEvent(int)));
}

void ButtonWaitDialog::updateText()
{
	setText(QString("Push ") + (m_longPress ? "and HOLD" : "") + " button on device to " + m_action + "\n\nTimeout in " + QString::number(m_timeLeft) + " seconds");
}

void ButtonWaitDialog::resetTimeout()
{
	m_timeLeft = sTimeoutPeriod;
	updateText();
}

void ButtonWaitDialog::signetdevTimerEvent(int val)
{
	m_timeLeft = val;
	if (!m_timeLeft) {
		done(QMessageBox::Cancel);
	}
	updateText();
}
