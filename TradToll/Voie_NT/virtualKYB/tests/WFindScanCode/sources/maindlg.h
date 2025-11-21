
#ifndef MAINDLG_H
#define MAINDLG_H

#include <QtGui>
#include <QMainWindow>
#include <QTimer>
#include "ui_MainDlg.h"


class MainDlg:public QMainWindow
{
    Q_OBJECT

public:
    MainDlg(QWidget *parent = 0);

public slots:
	void onClose();	
	void onBtnSimulatEvent();	
	void onBtnSimulateSeqEvents();	
	void onSequenceTimer();
	void onStopped();
	
protected:
	virtual void closeEvent(QCloseEvent * event);
	bool eventFilter(QObject *obj, QEvent *event);
	void setMessage(QString msg);
	bool keyPressEventHandler(QObject *target, QKeyEvent * event );
	void simulateKey(quint8 bVk, quint32 dwFlags);

private:
    Ui_MainDlg				ui;

	bool						m_bExit;

	quint8 m_bVk_Start;    
	quint8 m_bVk_End;   
	quint8 m_bVk_Current;
	bool m_bSequenceSimuStarted;
	bool m_bKeyDown;
	QTimer tmSequence;

};

#endif

