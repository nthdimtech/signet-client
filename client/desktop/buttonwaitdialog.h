#ifndef BUTTONWAITDIALOG_H
#define BUTTONWAITDIALOG_H

#include <QMessageBox>
#include <QTimer>

class QString;

class ButtonWaitDialog : public QMessageBox
{
	Q_OBJECT
	int m_timeLeft;
	void updateText();
	QString m_action;
	bool m_longPress;
	//TODO: We need to do away with this
	// constant. The device should decide
	// the timeout
	const static int sTimeoutPeriod = 10;
public:
	ButtonWaitDialog(QString title, QString action, QWidget *parent = nullptr, bool longPress = false);
	void resetTimeout();
public slots:
	void signetdevTimerEvent(int);
};

#endif // BUTTONWAITDIALOG_H
