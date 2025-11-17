#include <QtGui>
#include <QSpinBox>
#include <QTextCodec>
#include <QMessageBox>
#include <WINDOWS.H>

#include "maindlg.h"


MainDlg::MainDlg(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	m_bExit = false;

	QObject::connect(ui.btnSimulatEvent, SIGNAL(clicked()), this, SLOT(onBtnSimulatEvent()));
	QObject::connect(ui.btnSimulateSeqEvents, SIGNAL(clicked()), this, SLOT(onBtnSimulateSeqEvents()));
	QObject::connect(ui.btnClose, SIGNAL(clicked()), this, SLOT(onClose()));
	QObject::connect(ui.btnStop, SIGNAL(clicked()), this, SLOT(onStopped()));
	QObject::connect(&tmSequence, SIGNAL(timeout()), this, SLOT(onSequenceTimer()));
	
	ui.listWidget->setMaximumBlockCount(1000);

	ui.listWidget->installEventFilter(this);

}

void MainDlg::setMessage(QString msg)
{
//	int iMsgEvents = m_pSendReciveThread->decMsgsEvents();
	// print message to main window
	//QString sNewLine = QString("Queued msgs: %1 - ").arg(iMsgEvents) + msg ;
	ui.listWidget->appendPlainText(msg);
}


void MainDlg::closeEvent(QCloseEvent * event)
{
	m_bExit = true;
}



void MainDlg::onBtnSimulatEvent()
{

	QString sVirtCode = ui.txtVirtKeyCode->text();
	QString sFlags = ui.txtFlags->text();
	
	quint8 bVk = sVirtCode.toInt(); 
	bool ok;
	bVk = (quint8)sVirtCode.toInt(&ok, 16);    

	quint32 dwFlags;
	dwFlags = (quint32)sFlags.toInt(&ok, 16);    

	simulateKey(bVk,dwFlags);
}	

void MainDlg::onBtnSimulateSeqEvents()
{
	bool ok;

	QString sVirtCodeStart = ui.txtFrom->text();
	QString sVirtCodeEnd = ui.txtTo->text();
	
	m_bVk_Start = (quint8)sVirtCodeStart.toInt(&ok, 16);    
	m_bVk_End = (quint8)sVirtCodeEnd.toInt(&ok, 16);    

	int iDelay = ui.spinDelay->value();

	if(m_bVk_Start<m_bVk_End)
	{
		m_bVk_Current = m_bVk_Start;
		m_bSequenceSimuStarted = true;
		m_bKeyDown = true;
		tmSequence.setInterval(iDelay);
		tmSequence.start();
	}

}	

void MainDlg::onStopped()
{
	m_bSequenceSimuStarted = false;
}

void MainDlg::onSequenceTimer()
{
	if(m_bSequenceSimuStarted)
	{
		if(m_bVk_Current<=m_bVk_End )
		{
			if(m_bKeyDown)
				simulateKey(m_bVk_Current, 0x0000);//key down
			else
			{
				simulateKey(m_bVk_Current, 0x0002);//key up
				m_bVk_Current++;
			}
			
			m_bKeyDown = !m_bKeyDown;
		}
		else
		{
			m_bSequenceSimuStarted = false;
			tmSequence.stop();
		}
	}
	else
	{
		tmSequence.stop();
	}

}

void MainDlg::simulateKey(quint8 bVk, quint32 dwFlags)
{
	setMessage(QString("Windows VirtCode:0x%1 Flags:0x%2 ").arg((int)bVk,2,16,QChar('0')).arg((int)dwFlags,4,16,QChar('0')));
	ui.listWidget->setFocus();
	keybd_event(bVk, 0, dwFlags, 0);
}

void MainDlg::onClose()
{
	close();
}	

bool MainDlg::keyPressEventHandler(QObject *target, QKeyEvent * event )
{
	int iScanCode = event->nativeScanCode();
	int iVirtualCode = event->nativeVirtualKey();
	int iKey = event->key();
	QString s = event->text();
	unsigned char cAscii = 0;
	if(s.size()>0) 
		cAscii = (unsigned char)s.at(0).toAscii();

	QString msg = QString("KeyPressEvent: ScanCode:%1 VirtualCode:%2 Key:%3 Text:[%4] Ascii(%5)")
									.arg(iScanCode)
									.arg(iVirtualCode)
									.arg(iKey)
									.arg(s)
									.arg(cAscii);

	setMessage(msg);

	return false;
}


bool MainDlg::eventFilter(QObject *obj, QEvent *event)
{
	
	if(event->type() == QEvent::KeyPress) 
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		return keyPressEventHandler(obj, keyEvent);
	}
	else {
		
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}
