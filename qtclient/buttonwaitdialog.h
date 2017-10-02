#ifndef BUTTONWAITDIALOG_H
#define BUTTONWAITDIALOG_H

#include <QMessageBox>
#include <QTimer>

class QString;

class ButtonWaitDialog : public QMessageBox
{
	Q_OBJECT
	int m_timeLeft;
	QTimer m_timer;
	void updateText();
	QString m_action;
	bool m_longPress;
	const static int sTimeoutPeriod = 10;
public:
	ButtonWaitDialog(QString title, QString action, QWidget *parent = 0, bool longPress = false);
	void startTimer();
	void stopTimer();
public slots:
	void selfFinished(int result);
	void tick();
};

#endif // BUTTONWAITDIALOG_H
