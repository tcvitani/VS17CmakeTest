#include <QtGui>
#include <QSpinBox>
#include <QTextCodec>
#include <QMessageBox>

#include "SimuSendReciveThread.h"
#include "maindlg.h"


MainDlg::MainDlg(QString sBlName, QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	m_bExit = false;

	QObject::connect(ui.btnSendInitConfig, SIGNAL(clicked()), this, SLOT(onSendInitConfig()));
	QObject::connect(ui.btnSendAffMove, SIGNAL(clicked()), this, SLOT(onSendAffMove()));
	QObject::connect(ui.btnSendAffShow0, SIGNAL(clicked()), this, SLOT(onSendAffHide()));
	QObject::connect(ui.btnSendAffShow1, SIGNAL(clicked()), this, SLOT(onSendAffShow()));
	QObject::connect(ui.btnSendAffDEBUT, SIGNAL(clicked()), this, SLOT(onSendAffDEBUT()));
	QObject::connect(ui.btnSendAffFIN, SIGNAL(clicked()), this, SLOT(onSendAffFIN()));
	QObject::connect(ui.btnSendArret, SIGNAL(clicked()), this, SLOT(onSendArret()));
    QObject::connect(ui.btnClose, SIGNAL(clicked()), this, SLOT(onClose()));


	//Send recive thread
	m_pSendReciveThread = new SimuSendReciveThread();
	m_pSendReciveThread->initaialize(sBlName);
	m_pSendReciveThread->start();
	//ReadWrite thread
	QObject::connect(m_pSendReciveThread, SIGNAL(message(QString)), this, SLOT(onMessage(QString)));
	
	ui.listWidget->setMaximumBlockCount(1000);
}

void MainDlg::onMessage(QString msg)
{
//	int iMsgEvents = m_pSendReciveThread->decMsgsEvents();
	// print message to main window
	//QString sNewLine = QString("Queued msgs: %1 - ").arg(iMsgEvents) + msg ;
	ui.listWidget->appendPlainText(msg);
}



void MainDlg::onSendAffMove()
{
	m_pSendReciveThread->sendAffMove(ui.spinPosX->value(), ui.spinPosY->value());
}

void MainDlg::onSendAffHide()
{
	m_pSendReciveThread->sendAffHide();
}
void MainDlg::onSendAffShow()
{
	m_pSendReciveThread->sendAffShow();
}


void MainDlg::onSendInitConfig()
{
	m_pSendReciveThread->sendInitConfig(ui.txtKeyboardID->text());
}


void MainDlg::onSendAffDEBUT()
{
	m_pSendReciveThread->sendAffDEBUT();
}

void MainDlg::onSendAffFIN()
{
	m_pSendReciveThread->sendAffFIN();
}


void MainDlg::onSendArret()
{
	m_pSendReciveThread->sendArret();
}


void MainDlg::closeEvent(QCloseEvent * event)
{
	m_bExit = true;
}



void MainDlg::onClose()
{
	m_pSendReciveThread->sendArret();
	close();
}	

