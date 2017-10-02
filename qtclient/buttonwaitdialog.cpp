#include "buttonwaitdialog.h"

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
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
	connect(this, SIGNAL(finished(int)), this, SLOT(selfFinished(int)));
	m_timer.start(1000);
}

void ButtonWaitDialog::stopTimer()
{
	m_timer.stop();
}

void ButtonWaitDialog::startTimer()
{
	m_timeLeft = sTimeoutPeriod;
	m_timer.stop();
	m_timer.start(1000);
	updateText();
}

void ButtonWaitDialog::selfFinished(int result)
{
	Q_UNUSED(result);
	//We need this because due to the way modal dialogs are used in concert
	//with deleteLater() we might not delete the object right away but we
	//need to make sure we stop the timer
	m_timer.stop();
}

void ButtonWaitDialog::updateText()
{
	setText(QString("Push ") + (m_longPress ? "and HOLD" : "") + " button on device to " + m_action + "\n\nTimeout in " + QString::number(m_timeLeft) + " seconds");
}

void ButtonWaitDialog::tick()
{
	if (m_timeLeft) {
		m_timeLeft--;
	}
	if (!m_timeLeft) {
		done(QMessageBox::Cancel);
		m_timer.stop();
	}
	updateText();
}
