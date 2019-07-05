#ifndef BUTTONWAITWIDGET_H
#define BUTTONWAITWIDGET_H

#include <QMessageBox>
#include <QTimer>

class QString;
class QPushButton;

class ButtonWaitWidget : public QWidget
{
	Q_OBJECT
    QLabel *m_actionText;
    QLabel *m_timeoutText;
	QPushButton *m_cancelButton;
	int m_timeLeft;
	void updateText();
    QString m_action;
	bool m_longPress;
    bool m_timeoutOccured;
	//TODO: We need to do away with this
	// constant. The device should decide
	// the timeout
	const static int sTimeoutPeriod = 10;
public:
	ButtonWaitWidget(QString action, bool longPress = false, QWidget *parent = nullptr);
	void resetTimeout();
public slots:
	void signetdevTimerEvent(int);
signals:
	void timeout();
	void canceled();
};

#endif // BUTTONWAITDIALOG_H
