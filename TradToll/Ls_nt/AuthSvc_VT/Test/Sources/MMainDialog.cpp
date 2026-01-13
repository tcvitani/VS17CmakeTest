/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 spy_tool												 */
/* FILE:	 MMainDialog.cpp												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
#include <winsock2.h>
#include <windows.h>
#include <Iphlpapi.h>
#include <Lm.h>

#include <mbstring.h>
#include <QtGui>
#include <QFileDialog>
#include <QFileInfo>
#include <QUrl>
#include <QThread>
#include <QDesktopServices>
#include <QProcess>
#include <QDebug>
#include <cstring>
#include "MDefines.h"
#include "MGlobalStructs.h"
#include "MMainDialog.h"
#include "MTracer.h"
#include "MCmdACOM.h"
#include "MHelpFuncs.h"

extern "C"
{
	#include <reg.h>
	#include <msg_lc_auth_vt_enl_rep.h>
	#include <msg_lc_auth_vt_enl_req.h>
	#include <msg_lc_auth_vt_exl_rep.h>
	#include <msg_lc_auth_vt_exl_req.h>
	#include <msg_lc_auth_vt_enl_rep_v2.h>
	#include <msg_lc_auth_vt_enl_req_v2.h>
	#include <msg_lc_auth_vt_exl_rep_v2.h>
	#include <msg_lc_auth_vt_exl_req_v2.h>

};

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

MMainDialog::MMainDialog(QWidget *parent) :m_ui(new Ui::MMainDialogB), QDialog(parent)
{
	m_ui->setupUi(this);
	setAcceptDrops(true);

	SetupGui();

	QString sVer = MHelpFuncs::getVersionInfo();
	
	m_sTitle =CMDSVC_LBL_MAINDLG_TITLE + QString(" v.%1").arg(sVer);

	setWindowTitle(m_sTitle);

	// Disable autoDefault property on all buttons
	for each (QPushButton *button in this->findChildren<QPushButton*>())
		button->setAutoDefault(false);

	//------------------------------------------------
	m_pCmdACOM = new MCmdACOM(this);
	connect(m_pCmdACOM, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(m_pCmdACOM, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(m_pCmdACOM, SIGNAL(messageReceived(QByteArray)), this, SLOT(onACOMMessageReceived(QByteArray)));
	connect(m_pCmdACOM, SIGNAL(messageSent(QByteArray)), this, SLOT(onACOMMessageSent(QByteArray)));

	//------------------------------------------------
	connect(m_ui->btnConnect, SIGNAL(clicked()), this, SLOT(onBtnConnect_clicked()));
	connect(m_ui->btnDisconnect, SIGNAL(clicked()), this, SLOT(onBtnDisonnect_clicked()));
	
	connect(m_ui->btnSendCmd, SIGNAL(clicked()), this, SLOT(onBtnSendCmd_clicked()));
	connect(m_ui->btnClear, SIGNAL(clicked()), this, SLOT(onBtnClear_clicked()));
	connect(m_ui->btnBrowse, SIGNAL(clicked()), this, SLOT(onBtnBrowse_clicked()));
	connect(m_ui->chkRedirectToFile, SIGNAL(stateChanged(int)), this, SLOT(onchkRedirect_stateChanged(int)));
	connect(m_ui->txtCommand, SIGNAL(returnPressed()), this, SLOT(onCommand_returnPressed()));

	connect(m_ui->btnGenerateENL_REQ, SIGNAL(clicked()), this, SLOT(onBtnGenerateENL_REQ_clicked()));
	connect(m_ui->btnGenerateEXL_REQ, SIGNAL(clicked()), this, SLOT(onBtnGenerateEXL_REQ_clicked()));
	connect(m_ui->btnGenerateENL_REQ_2, SIGNAL(clicked()), this, SLOT(onBtnGenerateENL_REQ2_clicked()));
	connect(m_ui->btnGenerateEXL_REQ_2, SIGNAL(clicked()), this, SLOT(onBtnGenerateEXL_REQ2_clicked()));

	m_pfileCommLog = NULL;

	updateDialogStatus(enuDisconnected);

	QDateTime tmNow = QDateTime::currentDateTime();
	m_ui->dateTimeEntry_RefTime->setDateTime(tmNow);
	m_ui->dateTimeEntry_Time->setDateTime(tmNow);

	m_ui->dateTimeExit_RefTime->setDateTime(tmNow);
	m_ui->dateTimeExit_Time->setDateTime(tmNow);


}

MMainDialog::~MMainDialog()
{
	m_pCmdACOM->DisconnectACOM();
	delete m_pCmdACOM; m_pCmdACOM = NULL;	
	
	if (m_ui != NULL)
	{
		delete m_ui;
		m_ui = NULL;
	}

	if (m_pfileCommLog != NULL)
	{
		m_pfileCommLog->close();
		delete m_pfileCommLog; 
		m_pfileCommLog = NULL;
	}

}

//-------------------------------------------------------

void MMainDialog::onClose()
{
	close();
}

//-------------------------------------------------------

//------------------------ABOUT DIALOG-------------------------------

//------------------------READING-------------------------------
#define AUTHSVC_PIPE_COM            "\\\\%s\\pipe\\RouteSvc\\LS"

void MMainDialog::onBtnConnect_clicked()
{
	if (m_eDialogStatus == enuDisconnected)
	{ 
		openRedirectionFile();
		
		struct _ServiceCmdData sData;

		sData.sMachineName = m_ui->txtMachineName->text();
		sData.dwPort = 1;
		sData.sServiceName = "RouteSvc";
		sData.eInstanceType = enuACOM_PIPE_CLIENT;
		sData.sDisplayName = "NPipe:" + sData.sServiceName;
		sData.sInstanceFileName = QString("\\\\%1\\pipe\\RouteSvc\\LC").arg(sData.sMachineName); //connect to route service as LC

		m_pCmdACOM->ConnectACOM(&sData);
		updateDialogStatus(enuConnecting);

	}

		
}

void MMainDialog::onBtnDisonnect_clicked()
{
	if (m_eDialogStatus != enuDisconnected)
	{
		m_pCmdACOM->DisconnectACOM();
	}
}

void MMainDialog::onBtnBrowse_clicked()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
		m_ui->txtRedirtectToFile->text(),
		QString("*.log"));

	if (!fileName.isEmpty())
		m_ui->txtRedirtectToFile->setText(fileName);
}


void MMainDialog::onCommand_returnPressed()
{
	QByteArray baData = m_ui->txtCommand->text().toLocal8Bit();
	m_pCmdACOM->SendData(baData);
}

void MMainDialog::onBtnSendCmd_clicked()
{
	QByteArray baData = m_ui->txtCommand->text().toLocal8Bit();
	m_pCmdACOM->SendData(baData);
}

void MMainDialog::onBtnGenerateENL_REQ_clicked()
{
	struct MSG_LC_AUTH_VT_ENL_REQ * psAuthEnlReq = NULL;
	BYTE string_w[5000] = { 0 }; 
	DWORD size;

	psAuthEnlReq = MSG_LC_AUTH_VT_ENL_REQ_New();

	psAuthEnlReq->header.plaza_number = m_ui->txtEntry_plaza_id->text().toInt();
	psAuthEnlReq->header.lane_number = m_ui->txtEntry_lane_id->text().toInt(); 

	//psAuthEnlReq->body.dte_ref_entry	m_ui->txtEntry_RefTime->text();
	//GetLocalTime(&psAuthEnlReq->body.dte_ref_entry);

	QDateTime tm = m_ui->dateTimeEntry_RefTime->dateTime();
	MHelpFuncs::DateTime2SysTime(&tm, &psAuthEnlReq->body.dte_ref_entry);
	m_ui->dateTimeEntry_RefTime->setDateTime(QDateTime::currentDateTime());

	//psAuthEnlReq->body.dte_entry       m_ui->txtEntry_Time->text();
	//GetLocalTime(&psAuthEnlReq->body.dte_entry);
	QDateTime tm2 = m_ui->dateTimeEntry_Time->dateTime();
	MHelpFuncs::DateTime2SysTime(&tm, &psAuthEnlReq->body.dte_entry);
	m_ui->dateTimeEntry_Time->setDateTime(QDateTime::currentDateTime());


	psAuthEnlReq->body.entry_trs_ref_num = m_ui->txtEntry_trs_ref_num->text().toInt();
	m_ui->txtEntry_trs_ref_num->setText(QString("%1").arg(psAuthEnlReq->body.entry_trs_ref_num + 1));

	psAuthEnlReq->body.entry_plaza_id = m_ui->txtEntry_plaza_id->text().toInt();
	psAuthEnlReq->body.entry_lane_id = m_ui->txtEntry_lane_id->text().toInt();

	strcpy_s(psAuthEnlReq->body.entry_trs_id, sizeof(psAuthEnlReq->body.entry_trs_id), m_ui->txtEntry_trs_id->text().toAscii().data());

	strcpy_s(psAuthEnlReq->body.entry_vrn, sizeof(psAuthEnlReq->body.entry_vrn), m_ui->txtEntry_entry_vrn->text().toAscii().data());
	strcpy_s(psAuthEnlReq->body.entry_vrn_country, sizeof(psAuthEnlReq->body.entry_vrn_country), m_ui->txtEntry_entry_vrn_country->text().toAscii().data());
	strcpy_s(psAuthEnlReq->body.entry_provider, sizeof(psAuthEnlReq->body.entry_provider), m_ui->txtEntry_entry_provider->text().toAscii().data());

	if (MSG_LC_AUTH_VT_ENL_REQ_Write(psAuthEnlReq, string_w, sizeof(string_w), &size) == TRUE)
	{
		m_ui->txtCommand->setText((char*)string_w);
	}

	MSG_LC_AUTH_VT_ENL_REQ_Delete_All(psAuthEnlReq);

}

void MMainDialog::onBtnGenerateEXL_REQ_clicked()
{
	struct MSG_LC_AUTH_VT_EXL_REQ * psAuthExlReq = NULL;
	BYTE string_w[5000] = { 0 };
	DWORD size;

	psAuthExlReq = MSG_LC_AUTH_VT_EXL_REQ_New();

	psAuthExlReq->header.plaza_number = m_ui->txtExit_plaza_id->text().toInt();
	psAuthExlReq->header.lane_number = m_ui->txtExit_lane_id->text().toInt();

	//psAuthExlReq->body.dte_ref_exit	m_ui->txtExit_RefTime->text();
	//GetLocalTime(&psAuthExlReq->body.dte_ref_exit);
	QDateTime tm = m_ui->dateTimeExit_RefTime->dateTime();
	MHelpFuncs::DateTime2SysTime(&tm, &psAuthExlReq->body.dte_ref_exit);

	m_ui->dateTimeExit_RefTime->setDateTime(QDateTime::currentDateTime());

	//psAuthExlReq->body.dte_exit       m_ui->txtExit_Time->text();
	//GetLocalTime(&psAuthExlReq->body.dte_exit);
	QDateTime tm2 = m_ui->dateTimeExit_Time->dateTime();
	MHelpFuncs::DateTime2SysTime(&tm, &psAuthExlReq->body.dte_exit);
	m_ui->dateTimeExit_Time->setDateTime(QDateTime::currentDateTime());

	psAuthExlReq->body.exit_trs_ref_num = m_ui->txtExit_trs_ref_num->text().toInt();
	m_ui->txtExit_trs_ref_num->setText(QString("%1").arg(psAuthExlReq->body.exit_trs_ref_num + 1));

	psAuthExlReq->body.exit_plaza_id = m_ui->txtExit_plaza_id->text().toInt();
	psAuthExlReq->body.exit_lane_id = m_ui->txtExit_lane_id->text().toInt();

	strcpy_s(psAuthExlReq->body.exit_trs_id, sizeof(psAuthExlReq->body.exit_trs_id), m_ui->txtExit_trs_id->text().toAscii().data());

	strcpy_s(psAuthExlReq->body.exit_vrn, sizeof(psAuthExlReq->body.exit_vrn), m_ui->txtExit_exit_vrn->text().toAscii().data());
	strcpy_s(psAuthExlReq->body.exit_vrn_country, sizeof(psAuthExlReq->body.exit_vrn_country), m_ui->txtExit_exit_vrn_country->text().toAscii().data());
	strcpy_s(psAuthExlReq->body.exit_provider, sizeof(psAuthExlReq->body.exit_provider), m_ui->txtExit_exit_provider->text().toAscii().data());

	if (MSG_LC_AUTH_VT_EXL_REQ_Write(psAuthExlReq, string_w, sizeof(string_w), &size) == TRUE)
	{
		m_ui->txtCommand->setText((char*)string_w);
	}

	MSG_LC_AUTH_VT_EXL_REQ_Delete_All(psAuthExlReq);


}




void MMainDialog::onBtnGenerateENL_REQ2_clicked()
{
	struct MSG_LC_AUTH_VT_ENL_REQ_V2 * psAuthEnlReq = NULL;
	BYTE string_w[5000] = { 0 };
	DWORD size;

	psAuthEnlReq = MSG_LC_AUTH_VT_ENL_REQ_V2_New();

	psAuthEnlReq->header.plaza_number = m_ui->txtEntry_plaza_id->text().toInt();
	psAuthEnlReq->header.lane_number = m_ui->txtEntry_lane_id->text().toInt();

	//psAuthEnlReq->body.dte_ref_entry	m_ui->txtEntry_RefTime->text();
	//GetLocalTime(&psAuthEnlReq->body.dte_ref_entry);

	QDateTime tm = m_ui->dateTimeEntry_RefTime_2->dateTime();
	MHelpFuncs::DateTime2SysTime(&tm, &psAuthEnlReq->body.dte_ref_entry);
	m_ui->dateTimeEntry_RefTime_2->setDateTime(QDateTime::currentDateTime());

	//psAuthEnlReq->body.dte_entry       m_ui->txtEntry_Time->text();
	//GetLocalTime(&psAuthEnlReq->body.dte_entry);
	QDateTime tm2 = m_ui->dateTimeEntry_Time_2->dateTime();
	MHelpFuncs::DateTime2SysTime(&tm, &psAuthEnlReq->body.dte_entry);
	m_ui->dateTimeEntry_Time_2->setDateTime(QDateTime::currentDateTime());


	psAuthEnlReq->body.entry_trs_ref_num = m_ui->txtEntry_trs_ref_num_2->text().toInt();
	m_ui->txtEntry_trs_ref_num_2->setText(QString("%1").arg(psAuthEnlReq->body.entry_trs_ref_num + 1));

	psAuthEnlReq->body.entry_plaza_id = m_ui->txtEntry_plaza_id_2->text().toInt();
	psAuthEnlReq->body.entry_lane_id = m_ui->txtEntry_lane_id_2->text().toInt();

	strcpy_s(psAuthEnlReq->body.entry_trs_id, sizeof(psAuthEnlReq->body.entry_trs_id), m_ui->txtEntry_trs_id_2->text().toAscii().data());

	strcpy_s(psAuthEnlReq->body.entry_vrn, sizeof(psAuthEnlReq->body.entry_vrn), m_ui->txtEntry_entry_vrn_2->text().toAscii().data());
	strcpy_s(psAuthEnlReq->body.entry_vrn_country, sizeof(psAuthEnlReq->body.entry_vrn_country), m_ui->txtEntry_entry_vrn_country_2->text().toAscii().data());
	psAuthEnlReq->body.ticket_type = (DWORD)m_ui->spinEntry_Ticket_Type->value();

	if (MSG_LC_AUTH_VT_ENL_REQ_V2_Write(psAuthEnlReq, string_w, sizeof(string_w), &size) == TRUE)
	{
		m_ui->txtCommand->setText((char*)string_w);
	}

	MSG_LC_AUTH_VT_ENL_REQ_V2_Delete_All(psAuthEnlReq);

}

void MMainDialog::onBtnGenerateEXL_REQ2_clicked()
{
	struct MSG_LC_AUTH_VT_EXL_REQ_V2 * psAuthExlReq = NULL;
	BYTE string_w[5000] = { 0 };
	DWORD size;

	psAuthExlReq = MSG_LC_AUTH_VT_EXL_REQ_V2_New();

	psAuthExlReq->header.plaza_number = m_ui->txtExit_plaza_id_2->text().toInt();
	psAuthExlReq->header.lane_number = m_ui->txtExit_lane_id_2->text().toInt();

	//psAuthExlReq->body.dte_ref_exit	m_ui->txtExit_RefTime->text();
	//GetLocalTime(&psAuthExlReq->body.dte_ref_exit);
	QDateTime tm = m_ui->dateTimeExit_RefTime_2->dateTime();
	MHelpFuncs::DateTime2SysTime(&tm, &psAuthExlReq->body.dte_ref_exit);

	m_ui->dateTimeExit_RefTime_2->setDateTime(QDateTime::currentDateTime());

	//psAuthExlReq->body.dte_exit       m_ui->txtExit_Time->text();
	//GetLocalTime(&psAuthExlReq->body.dte_exit);
	QDateTime tm2 = m_ui->dateTimeExit_Time_2->dateTime();
	MHelpFuncs::DateTime2SysTime(&tm, &psAuthExlReq->body.dte_exit);
	m_ui->dateTimeExit_Time_2->setDateTime(QDateTime::currentDateTime());

	psAuthExlReq->body.exit_trs_ref_num = m_ui->txtExit_trs_ref_num_2->text().toInt();
	m_ui->txtExit_trs_ref_num_2->setText(QString("%1").arg(psAuthExlReq->body.exit_trs_ref_num + 1));

	psAuthExlReq->body.exit_plaza_id = m_ui->txtExit_plaza_id_2->text().toInt();
	psAuthExlReq->body.exit_lane_id = m_ui->txtExit_lane_id_2->text().toInt();

	strcpy_s(psAuthExlReq->body.exit_trs_id, sizeof(psAuthExlReq->body.exit_trs_id), m_ui->txtExit_trs_id_2->text().toAscii().data());

	strcpy_s(psAuthExlReq->body.exit_vrn, sizeof(psAuthExlReq->body.exit_vrn), m_ui->txtExit_exit_vrn_2->text().toAscii().data());
	strcpy_s(psAuthExlReq->body.exit_vrn_country, sizeof(psAuthExlReq->body.exit_vrn_country), m_ui->txtExit_exit_vrn_country_2->text().toAscii().data());
	psAuthExlReq->body.flag_eticket = (DWORD)m_ui->spinExit_Flag_ETicket->value();
	psAuthExlReq->body.ticket_type = (DWORD)m_ui->spinExit_Ticket_Type->value();

	if (MSG_LC_AUTH_VT_EXL_REQ_V2_Write(psAuthExlReq, string_w, sizeof(string_w), &size) == TRUE)
	{
		m_ui->txtCommand->setText((char*)string_w);
	}

	MSG_LC_AUTH_VT_EXL_REQ_V2_Delete_All(psAuthExlReq);


}
































void MMainDialog::onBtnClear_clicked()
{
	m_ui->txtOutput->clear();
}

void MMainDialog::openRedirectionFile()
{
	if (m_ui->chkRedirectToFile->checkState() == Qt::Checked && m_pfileCommLog == NULL)
	{
		QString sFilePath = m_ui->txtRedirtectToFile->text();
		
		//test to see if the path is valid
		m_pfileCommLog = new QFile(sFilePath);
		if (m_pfileCommLog->open(QIODevice::WriteOnly))
		{
			m_pfileCommLog->write(QString("------- Start log ---- ").toLocal8Bit());
			;//... continue with redirection to the file
		}
		else
		{
			delete m_pfileCommLog; 
			m_pfileCommLog = NULL;

			raiseMsgBox(this, QString("Invalid redirect file path:[%1]!").arg(sFilePath), tr("LABEL_OK"), QString(), QMessageBox::Warning);
			m_ui->chkRedirectToFile->setCheckState(Qt::Unchecked);
			//Error
		}
	}
}


void MMainDialog::onchkRedirect_stateChanged(int state)
{
	if (state == Qt::Checked)
	{
		openRedirectionFile();
	}

}



void MMainDialog::updateDialogStatus(enumDialogStatus eDialogStatus)
{
	m_eDialogStatus = eDialogStatus;

	if (m_ui == NULL)
		return;
	

	if (m_eDialogStatus == enuDisconnected)
	{
		m_ui->btnConnect->setEnabled(true);
		m_ui->btnDisconnect->setEnabled(false);
		m_ui->btnBrowse->setEnabled(true);
		m_ui->chkRedirectToFile->setEnabled(true);
		m_ui->txtMachineName->setEnabled(true);
		m_ui->txtRedirtectToFile->setEnabled(true);

		m_ui->txtCommand->setEnabled(false);
		m_ui->btnSendCmd->setEnabled(false);

		m_ui->lblStatus->setText(tr(CMDSVC_LBL_STATUS_DISCONNECTED));
		m_ui->lblLinkFail->setPixmap(QPixmap(QString::fromUtf8(":/Images/ledoff.png")));

	}
	else if (m_eDialogStatus == enuConnecting)
	{
		m_ui->btnConnect->setEnabled(false);
		m_ui->btnDisconnect->setEnabled(true);
		m_ui->btnBrowse->setEnabled(false);
		m_ui->chkRedirectToFile->setEnabled(false);
		m_ui->txtMachineName->setEnabled(false);
		m_ui->txtRedirtectToFile->setEnabled(false);

		m_ui->txtCommand->setEnabled(false);
		m_ui->btnSendCmd->setEnabled(false);

		m_ui->lblStatus->setText(tr(CMDSVC_LBL_STATUS_CONNECTING));
		m_ui->lblLinkFail->setPixmap(QPixmap(QString::fromUtf8(":/Images/ledred.png")));

	}
	else if (m_eDialogStatus == enuConnected)
	{
		m_ui->btnConnect->setEnabled(false);
		m_ui->btnDisconnect->setEnabled(true);
		m_ui->btnBrowse->setEnabled(false);
		m_ui->chkRedirectToFile->setEnabled(false);
		m_ui->txtMachineName->setEnabled(false);
		m_ui->txtRedirtectToFile->setEnabled(false);

		m_ui->txtCommand->setEnabled(true);
		m_ui->btnSendCmd->setEnabled(true);

		m_ui->lblStatus->setText(tr(CMDSVC_LBL_STATUS_CONNECTED));
		m_ui->lblLinkFail->setPixmap(QPixmap(QString::fromUtf8(":/Images/ledgreen.png")));
	}
}


//---------------------------STYLE----------------------------
void MMainDialog::SetupGui()
{

}

void MMainDialog::onMessage(const QString &s)
{
	displayMessage(s);
}

void MMainDialog::displayMessage(QString sMsg)
{
	m_ui->txtOutput->appendPlainText(sMsg);
}

void MMainDialog::displayMessageIN(QString sMsg)
{
	sMsg.prepend("  [").append("]");

	m_ui->txtOutput->appendPlainText(sMsg);
}

void MMainDialog::displayMessageOUT(QString sMsg)
{
	sMsg.prepend("COMMAND: [").append("]");

	m_ui->txtOutput->appendPlainText(sMsg);
}

bool MMainDialog::raiseMsgBox(QWidget *pParent,
	QString sMessage,
	QString sBtnOkText,
	QString sBtnCancelText,
	int iMsgBoxIcon)
{
	QMessageBox		msgBox(pParent);
	int				iMsgBoxRet = 0;
	bool			bRet = false;

	msgBox.setIcon((QMessageBox::Icon)iMsgBoxIcon);
	msgBox.setText(sMessage);
	msgBox.setWindowTitle(pParent->windowTitle());

	if (!sBtnOkText.isEmpty())
	{
		if (!sBtnCancelText.isEmpty())
		{
			msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			msgBox.setButtonText(QMessageBox::Cancel, QString(sBtnCancelText));
			msgBox.setDefaultButton(QMessageBox::Cancel);
		}
		else
			msgBox.setStandardButtons(QMessageBox::Ok);

		msgBox.setButtonText(QMessageBox::Ok, QString(sBtnOkText));
	}

	msgBox.setWindowFlags(msgBox.windowFlags() & ~Qt::WindowTitleHint);

	iMsgBoxRet = msgBox.exec();
	switch (iMsgBoxRet)
	{
	case QMessageBox::Ok:
		bRet = true;
		break;

	case QMessageBox::Cancel:
		bRet = false;
		break;
	}

	return bRet;
}



void MMainDialog::onConnected()
{
	updateDialogStatus(enuConnected);

	displayMessage("CONNECTED TO SERVICE");

	//struct MSG_SV_CON_REP * psConRep = NULL;
	//psConRep = MSG_SV_CON_REP_New();
	//MSG_SV_CON_REP_Write(struct MSG_SV_CON_REP *p_ptr,
	//	BYTE *p_msg,
	//	DWORD msg_size_max,
	//	DWORD *final_msg_size);


	//m_pCmdACOM->SendData(p_msg);


}

void MMainDialog::onDisconnected()
{

	updateDialogStatus(enuDisconnected);

	if (m_pfileCommLog != NULL)
	{
		m_pfileCommLog->flush();
		m_pfileCommLog->close();
		delete m_pfileCommLog;
		m_pfileCommLog = NULL;
	}

}

void MMainDialog::onACOMMessageReceived(QByteArray ba)
{
	if (m_pfileCommLog != NULL)
	{
		m_pfileCommLog->write(ba);
		m_pfileCommLog->write("\n");
		m_pfileCommLog->flush();
	}

	//if (ba.indexOf("PROMPTBEGIN") >= 0)
	//	;
	//else if (ba.indexOf("PROMPTITEM") >= 0)	 //ElseIf InStr(1, szMsg, "PROMPTITEM") = 1 Then		AddText Trim(Mid(szMsg, 11))
	//	displayMessage(ba.mid(11));
	//else if (ba.indexOf("PROMPTEND") >= 0)  //ElseIf InStr(1, szMsg, "PROMPTEND") = 1 Then
	//	displayMessage("---------------------");
	//else if (ba.indexOf("COMMANDLISTITEM") >= 0) //ElseIf InStr(1, szMsg, "COMMANDLISTITEM") = 1 Then	AddCommand Trim(Mid(szMsg, 16))
	//{
	//	QByteArray baCmd = ba.mid(16);
	//	QString sCmd;

	//	int iPos = baCmd.indexOf(',');
	//	if (iPos > 0 && iPos<baCmd.size()-1)
	//	{
	//		sCmd = QString("%1 %2").arg(baCmd.left(iPos).data()).arg(baCmd.mid(iPos + 1).data());
	//	}
	//	else
	//		sCmd = baCmd.trimmed();

	//	sCmd.replace("|", ",");
	//	m_ui->cmbCommandList->addItem(sCmd);
	//
	//	displayMessageIN(ba.mid(16));
	//}
	//else if (ba.indexOf("COMMANDLISTEND") >= 0)	 //ElseIf InStr(1, szMsg, "COMMANDLISTEND") = 1 Then
	//	;
	//else
		displayMessageIN(ba);
}

void MMainDialog::onACOMMessageSent(QByteArray ba)
{
	displayMessageOUT(ba);
}


/*-------------------------------- END OF FILE ------------------------------*/