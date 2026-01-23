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
#include <QMessageBox>
#include <cstring>
#include "MDefines.h"
#include "MGlobalStructs.h"
#include "MMainDialog.h"
#include "MTranslator.h"
#include "MCmdSvcToolConfig.h"
#include "MAboutWidget.h"
#include "MStyledToolBox.h"
#include "MStyledLangPick.h"
#include "MStyledTitleBar.h"
#include "MLangPickConfig.h"
#include "MTracer.h"
#include "MCmdACOM.h"

extern "C"
{
	#include <reg.h>
};

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

MMainDialog::MMainDialog(QWidget *parent) :m_ui(new Ui::MMainDialogB), m_pAboutDlg(NULL), QDialog(parent)
{
	m_ui->setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	setAcceptDrops(true);

	SetupGui();

	QString str;
	str = MCmdSvcToolConfig::getCfg()->getWindowIconPath();

	QIcon ico = QIcon(str);
	
	if (!ico.isNull())
		setWindowIcon(ico);
	else
	{
		ico = QIcon(":\\Images\\CmdSvc_Tool.png");
		if (!ico.isNull())
			setWindowIcon(ico);
	}

	QString sVer = MHelpFuncs::getVersionInfo();
	
	m_sTitle =CMDSVC_LBL_MAINDLG_TITLE + QString(" v.%1").arg(sVer);

	setWindowTitle(m_sTitle);

	//Init window styled controls...
	//--Title bar
	m_pTitle = new MStyledTitleBar(this, m_ui->frTitle, m_ui->btnMinimize, m_ui->btnRestoreMax, m_ui->btnClose);
	m_pTitle->setButtonsPixmaps(QPixmap(QString::fromUtf8(":/Images/maximize.png")),
								QPixmap(QString::fromUtf8(":/Images/minimize.png")),
								QPixmap(QString::fromUtf8(":/Images/restore.png")),
								QPixmap() );

	connect(m_pTitle, SIGNAL(dialogResized()), this, SLOT(onDialogResized()));

    //--Languages menu...
	createLangPick();
	m_pLangPick->setSelectedLang(MConfig::getCfg()->getDefaultLanguageId());

	QByteArray baLanguageId = MConfig::getCfg()->getDefaultLanguageId().toLatin1();

	m_pLanguage = baLanguageId.data();


	//--About window ...
	createAboutDlg();
	connect(m_ui->btnAbout, SIGNAL(released()), this, SLOT(onBtnAbout()));
	
	
	// Load stylesheet file if path to that file isn't empty and if that file isn't empty.
	if (!MCmdSvcToolConfig::getCfg()->getStylesheetPath().isEmpty() && !MHelpFuncs::loadFileContent(MCmdSvcToolConfig::getCfg()->getStylesheetPath()).isEmpty())
		MHelpFuncs::setFileCSSToWidget(MCmdSvcToolConfig::getCfg()->getStylesheetPath(), this);

	// init configuration


	// Disable autoDefault property on all buttons
	foreach (QPushButton *button, this->findChildren<QPushButton*>())
		button->setAutoDefault(false);

	// Resize window if valid value of WindowSize is set in .ini file
	QString sWndSizeTemp = MCmdSvcToolConfig::getCfg()->getWindowSize();
	if (!sWndSizeTemp.isEmpty())
	{
		int iWidth		= 0;
		int iHeight		= 0;
		bool bWidthOk	= false;
		bool bHeightOk	= false;
		
		QStringList qsWndSizeList = sWndSizeTemp.split("x", Qt::SkipEmptyParts, Qt::CaseInsensitive);
		
		iWidth = qsWndSizeList.at(0).toInt(&bWidthOk);
		iHeight = qsWndSizeList.at(1).toInt(&bHeightOk);
		
		if (qsWndSizeList.count() == 2 && bWidthOk && bHeightOk && iWidth>0 && iHeight>0)
			resize (iWidth, iHeight);
	}
	
	// Move window if valid value of WindowPosition is set in .ini file
	QString sWndPosTemp = MCmdSvcToolConfig::getCfg()->getWindowPosition();
	if (!sWndPosTemp.isEmpty())
	{
		int iX		= 0;
		int iY		= 0;
		bool bXOk	= false;
		bool bYOk	= false;
		
		QStringList sWndPosList = sWndPosTemp.split(".", Qt::SkipEmptyParts, Qt::CaseInsensitive);
		
		iX = sWndPosList.at(0).toInt(&bXOk);
		iY = sWndPosList.at(0).toInt(&bYOk);
		
		if (sWndPosList.count() == 2 && bXOk && bYOk)
			move (iX, iY);
	}
	//------------------------------------------------
	m_pCmdACOM = new MCmdACOM(this);
	connect(m_pCmdACOM, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(m_pCmdACOM, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(m_pCmdACOM, SIGNAL(messageReceived(QByteArray)), this, SLOT(onACOMMessageReceived(QByteArray)));
	connect(m_pCmdACOM, SIGNAL(messageSent(QByteArray)), this, SLOT(onACOMMessageSent(QByteArray)));

	//------------------------------------------------
	connect(m_ui->btnConnect, SIGNAL(clicked()), this, SLOT(onBtnConnect_clicked()));
	connect(m_ui->btnDisconnect, SIGNAL(clicked()), this, SLOT(onBtnDisonnect_clicked()));
	

	connect(m_ui->btnRefresh, SIGNAL(clicked()), this, SLOT(onBtnRefresh_clicked()));
	connect(m_ui->btnSendCmd, SIGNAL(clicked()), this, SLOT(onBtnSendCmd_clicked()));
	connect(m_ui->btnClear, SIGNAL(clicked()), this, SLOT(onBtnClear_clicked()));
	connect(m_ui->btnBrowse, SIGNAL(clicked()), this, SLOT(onBtnBrowse_clicked()));
	connect(m_ui->chkRedirectToFile, SIGNAL(stateChanged(int)), this, SLOT(onchkRedirect_stateChanged(int)));
	connect(m_ui->cmbCommandList, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentCmdIndexChanged(int)));

	connect(m_ui->txtMachineName, SIGNAL(editingFinished()), this, SLOT(onMachineName_editingFinished()));
	connect(m_ui->txtMachineName, SIGNAL(textChanged(const QString &)), this, SLOT(onMachineName_textChanged(const QString &)));
	connect(m_ui->txtCommand, SIGNAL(returnPressed()), this, SLOT(onCommand_returnPressed()));



	m_ui->txtMachineName->setText(".");
	m_pfileCommLog = NULL;

	updateDialogStatus(enuNoServicesEnumerated);
	refreshServiceList();
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

void MMainDialog::createAboutDlg()
{
	m_pAboutDlg = new MAboutWidget(this);
	m_pAboutDlg->setObjectName(QString::fromUtf8("m_pAboutDlg"));
	
	m_pAboutDlg->hide();

	m_pAboutDlg->setLogoPix(MCmdSvcToolConfig::getCfg()->getLogoPixPath());
	m_pAboutDlg->setAppVersion(MHelpFuncs::getVersionInfo());
}

void MMainDialog::onBtnAbout()
{
	if (m_pAboutDlg->isVisible())	
		m_pAboutDlg->hide();
	else
	{
		updateAboutDlgSizeAndPos();
		m_pAboutDlg->show();
	}
}

void MMainDialog::updateAboutDlgSizeAndPos()
{
	QPoint ptContentPos = m_ui->frContent->pos(); //target position - (relative to parent "frBody")
	QPoint pt = m_ui->frBody->mapTo(this, ptContentPos); //top left of the target container
	
	int iHeightBody = m_ui->frContent->height();		
	int iWidthBody = m_ui->frContent->width();		

	int iWidth = m_pAboutDlg->width();		//get the width set in designer
	QSize size = QSize(iWidth, iHeightBody);
	
	m_pAboutDlg->resize(size);
	pt = QPoint(iWidthBody - size.width(),pt.y() ); //align right in the target container
	m_pAboutDlg->move(pt);
}

void MMainDialog::onDialogResized()
{
	updateAboutDlgSizeAndPos();
}


//----------------------------LANGUAGE-----------------------------------

void MMainDialog::createLangPick()
{
	m_pLangPick = new MStyledLangPick(m_ui->frMenuLangPick);
	QList <MLangPickItemConfigData*> *pLst = MCmdSvcToolConfig::getCfg()->getLangPickConfig()->getLangPickCfgList();

//	MTranslator* pTr = MConfigMainIntf::getCfg()->getTranslator();
	
	foreach(MLangPickItemConfigData* p, *pLst)
	{
		//if(pTr->isAvailable(p->m_sLangId))
			m_pLangPick->appendLanguage(p->m_sLangSelectingIMG, p->m_sLangSelectedIMG, p->m_sLangShortLabel, p->m_sLangId);
	}

	connect(m_pLangPick, SIGNAL(selectionChanged(QString)), this, SLOT(onLangSelectionChanged(QString)));
	connect(m_ui->btnLanguages, SIGNAL(released()), m_pLangPick,SLOT(toogleSelectingState()));
		
	QHBoxLayout * pLayout = (QHBoxLayout *)m_ui->frMenuLangPick->layout();
	if(pLayout!=NULL)
		pLayout->insertWidget(0,m_pLangPick);

	m_pLangPick->setSelectedLang(MConfig::getCfg()->getDefaultLanguageId());

}

void MMainDialog::onLangSelectionChanged(QString sLangID)
{
	m_languageName = sLangID;
	MTranslator* pTr = MCmdSvcToolConfig::getCfg()->getTranslator();
	pTr->setLanguage(m_languageName);

	QCoreApplication::removeTranslator(pTr);
	QCoreApplication::installTranslator(pTr);
}

void MMainDialog::changeEvent(QEvent* event)
{
   if (event->type() == QEvent::LanguageChange)
   {
       // retranslate designer form (single inheritance approach)
       m_ui->retranslateUi(this);
    
       // retranslate other widgets which weren't added in designer
       retranslate();
   }
   
   // remember to call base class implementation
   QDialog::changeEvent(event);
}

// To be used to retranslate other widgets which weren't added in designer
void MMainDialog::retranslate()
{
    m_pAboutDlg->retranslate();
}

//------------------------READING-------------------------------


bool MMainDialog::ExtractServiceList(QString sMachineName)
{
	bool bOK = false;
	m_ui->cmbServiceName->clear();
	mapServiceCmdData.clear();

	QString sServerName = sMachineName;
	if(sServerName == ".") 
			sServerName = "";

	QString sCMDSVC_KEY_SERVICES = "SYSTEM\\CurrentControlSet\\Services";
	QString sCMDSVC_KEY_PARAMS = "Parameters";
	QString sCMDSVC_VAL_DISPNAME = "DisplayName";
	QString sCMDSVC_VAL_ENABLED = "CmdSvcEnabled";
	HKEY hKey = NULL;

	DWORD lRes = REG_Ouvrir(sServerName.toLocal8Bit().data(), HKEY_LOCAL_MACHINE, sCMDSVC_KEY_SERVICES.toLocal8Bit().data(), &hKey);

	if (ERROR_SUCCESS == lRes)
	{
		bool bStop = false;
		long lIndex = 0;
		char szKeyName[MAX_PATH * 4] = { 0 };
		DWORD dwKeyNameSize;

		while (!bStop)
		{
			memset(szKeyName, 0, sizeof(szKeyName));
			dwKeyNameSize = sizeof(szKeyName) - 1;

			lRes = REG_Enum_Cles(hKey, (char*)"", lIndex, szKeyName, &dwKeyNameSize);
			if (ERROR_SUCCESS == lRes)
			{
				char szName[MAX_PATH * 4] = { 0 };
				DWORD dwNameSize = sizeof(szName) - 1;
				QString sServiceName;

				lRes = REG_Lire_Chaine(hKey, szKeyName, sCMDSVC_VAL_DISPNAME.toLocal8Bit().data(), szName, &dwNameSize);
				
				if (ERROR_SUCCESS == lRes)
				{
					sServiceName = szKeyName;

					DWORD lPort;
					QString sSvcRegKeyName;
					sSvcRegKeyName = QString(szKeyName) + QString("\\") + sCMDSVC_KEY_PARAMS;

					lRes = REG_Lire_Entier(hKey, sSvcRegKeyName.toLocal8Bit().data(), sCMDSVC_VAL_ENABLED.toLocal8Bit().data(), &lPort);
					if (ERROR_SUCCESS == lRes)
					{
						struct _ServiceCmdData sData;
						sData.sMachineName = (sMachineName == "") ? "." : sMachineName;
						sData.sServiceName = sServiceName;
						sData.dwPort = lPort;

						if (lPort == 1)
						{
							sData.eInstanceType = enuACOM_PIPE_CLIENT;
							sData.sDisplayName = "NPipe:" + sServiceName;
							sData.sInstanceFileName = QString("\\\\%1\\pipe\\%2\\CMD").arg(sData.sMachineName).arg(sServiceName);
							
						}
						else
						{
							sData.eInstanceType = enuACOM_TCP_CLIENT;
							sData.sDisplayName = "TCP-" + QString::number(lPort) + ":" + sServiceName;
							
							if ((sData.sMachineName == ".") || (sData.sMachineName == ""))
								sData.sInstanceFileName = QString("127.0.0.1:%1").arg(lPort);
							else
								sData.sInstanceFileName = QString("%1:%2").arg(sData.sMachineName).arg(lPort);
						}

						m_ui->cmbServiceName->addItem(sData.sDisplayName);
						mapServiceCmdData[sData.sDisplayName] = sData;
						bOK = true;
					}
				}

				lIndex = lIndex + 1;
			}
			else
				bStop = true;
		}
			
	}

	if (hKey!=0)
		REG_Fermer(hKey);



	return bOK;
}

bool MMainDialog::refreshServiceList()
{
	bool bOK = false;

	if (m_eDialogStatus == enuNoServicesEnumerated)
	{
		QString sServerName = m_ui->txtMachineName->text();

		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		
		if (ExtractServiceList(sServerName))
		{
			updateDialogStatus(enuDisconnected);
			bOK = true;
		}

		QApplication::restoreOverrideCursor();
	}

	return bOK;
}


void MMainDialog::onBtnRefresh_clicked()
{
	if (m_eDialogStatus != enuNoServicesEnumerated)
	{
		m_ui->cmbServiceName->clear();
		mapServiceCmdData.clear();
		updateDialogStatus(enuNoServicesEnumerated);
	}

	if (!refreshServiceList())
	{
		raiseMsgBox(this, tr("Service list not found!"), tr("LABEL_OK"), QString(), QMessageBox::Warning);
	}

}




void MMainDialog::onBtnConnect_clicked()
{
	if (m_eDialogStatus == enuDisconnected)
	{ 
		openRedirectionFile();
		QString sCurrentService = m_ui->cmbServiceName->currentText();
		QMap<QString, struct _ServiceCmdData>::const_iterator i = mapServiceCmdData.find(sCurrentService);

		if (i != mapServiceCmdData.end() && i.key() == sCurrentService)
		{
			struct _ServiceCmdData sData = i.value();
			m_pCmdACOM->ConnectACOM(&sData);
			updateDialogStatus(enuConnecting);
		}
		else
		{
			raiseMsgBox(this, tr("Selected service not found!"), tr("LABEL_OK"), QString(), QMessageBox::Warning);
		}
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


void MMainDialog::onCurrentCmdIndexChanged(int index)
{
	if (index > 0)
	{
		QString sNewCmd = m_ui->cmbCommandList->currentText();
		m_ui->txtCommand->setText(sNewCmd);
		m_ui->cmbCommandList->setCurrentIndex(0);
	}
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

void MMainDialog::onMachineName_editingFinished()
{
	//refreshServiceList();

}


void MMainDialog::onMachineName_textChanged(const QString & text)
{
	if (m_eDialogStatus != enuNoServicesEnumerated)
	{
		m_ui->cmbServiceName->clear();
		mapServiceCmdData.clear();
		updateDialogStatus(enuNoServicesEnumerated);
	}
}




void MMainDialog::updateDialogStatus(enumDialogStatus eDialogStatus)
{
	m_eDialogStatus = eDialogStatus;

	if (m_ui == NULL)
		return;
	

	if (m_eDialogStatus == enuNoServicesEnumerated)
	{
		m_ui->btnConnect->setEnabled(false);
		m_ui->btnDisconnect->setEnabled(false);
		m_ui->btnRefresh->setEnabled(true);
		m_ui->btnBrowse->setEnabled(true);
		m_ui->chkRedirectToFile->setEnabled(true);
		m_ui->txtMachineName->setEnabled(true);
		m_ui->cmbServiceName->setEnabled(false);
		m_ui->txtRedirtectToFile->setEnabled(true);

		m_ui->txtCommand->setEnabled(false);
		m_ui->cmbCommandList->setEnabled(false);
		m_ui->btnSendCmd->setEnabled(false);

		m_ui->lblStatus->setText(tr(CMDSVC_LBL_STATUS_ENUMERATING));
		m_ui->lblLinkFail->setPixmap(QPixmap(QString::fromUtf8(":/Images/ledoff.png")));

	}
	else if (m_eDialogStatus == enuDisconnected)
	{
		m_ui->btnConnect->setEnabled(true);
		m_ui->btnDisconnect->setEnabled(false);
		m_ui->btnRefresh->setEnabled(true);
		m_ui->btnBrowse->setEnabled(true);
		m_ui->chkRedirectToFile->setEnabled(true);
		m_ui->txtMachineName->setEnabled(true);
		m_ui->cmbServiceName->setEnabled(true);
		m_ui->txtRedirtectToFile->setEnabled(true);

		m_ui->txtCommand->setEnabled(false);
		m_ui->cmbCommandList->setEnabled(false);
		m_ui->btnSendCmd->setEnabled(false);

		m_ui->lblStatus->setText(tr(CMDSVC_LBL_STATUS_DISCONNECTED));
		m_ui->lblLinkFail->setPixmap(QPixmap(QString::fromUtf8(":/Images/ledoff.png")));

	}
	else if (m_eDialogStatus == enuConnecting)
	{
		m_ui->btnConnect->setEnabled(false);
		m_ui->btnDisconnect->setEnabled(true);
		m_ui->btnRefresh->setEnabled(false);
		m_ui->btnBrowse->setEnabled(false);
		m_ui->chkRedirectToFile->setEnabled(false);
		m_ui->txtMachineName->setEnabled(false);
		m_ui->cmbServiceName->setEnabled(false);
		m_ui->txtRedirtectToFile->setEnabled(false);

		m_ui->txtCommand->setEnabled(false);
		m_ui->cmbCommandList->setEnabled(false);
		m_ui->btnSendCmd->setEnabled(false);

		m_ui->lblStatus->setText(tr(CMDSVC_LBL_STATUS_CONNECTING));
		m_ui->lblLinkFail->setPixmap(QPixmap(QString::fromUtf8(":/Images/ledred.png")));

	}
	else if (m_eDialogStatus == enuConnected)
	{
		m_ui->btnConnect->setEnabled(false);
		m_ui->btnDisconnect->setEnabled(true);
		m_ui->btnRefresh->setEnabled(false);
		m_ui->btnBrowse->setEnabled(false);
		m_ui->chkRedirectToFile->setEnabled(false);
		m_ui->txtMachineName->setEnabled(false);
		m_ui->cmbServiceName->setEnabled(false);
		m_ui->txtRedirtectToFile->setEnabled(false);

		m_ui->txtCommand->setEnabled(true);
		m_ui->cmbCommandList->setEnabled(true);
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

	MHelpFuncs::setFileCSSToWidget(MCmdSvcToolConfig::getCfg()->getDialogStylesheetPath(), &msgBox);

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

	m_pCmdACOM->SendData("PROMPT");
	m_pCmdACOM->SendData("COMMANDLIST");

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

	if (ba.indexOf("PROMPTBEGIN") >= 0)
		;
	else if (ba.indexOf("PROMPTITEM") >= 0)	 //ElseIf InStr(1, szMsg, "PROMPTITEM") = 1 Then		AddText Trim(Mid(szMsg, 11))
		displayMessage(ba.mid(11));
	else if (ba.indexOf("PROMPTEND") >= 0)  //ElseIf InStr(1, szMsg, "PROMPTEND") = 1 Then
		displayMessage("---------------------");
	else if (ba.indexOf("COMMANDLISTBEGIN") >= 0)	//ElseIf InStr(1, szMsg, "COMMANDLISTBEGIN") = 1 Then	CommandCmb.Clear
	{
		m_ui->cmbCommandList->clear();
		m_ui->cmbCommandList->addItem("Pick a command...");
	}
	else if (ba.indexOf("COMMANDLISTITEM") >= 0) //ElseIf InStr(1, szMsg, "COMMANDLISTITEM") = 1 Then	AddCommand Trim(Mid(szMsg, 16))
	{
		QByteArray baCmd = ba.mid(16);
		QString sCmd;

		int iPos = baCmd.indexOf(',');
		if (iPos > 0 && iPos<baCmd.size()-1)
		{
			sCmd = QString("%1 %2").arg(baCmd.left(iPos).data()).arg(baCmd.mid(iPos + 1).data());
		}
		else
			sCmd = baCmd.trimmed();

		sCmd.replace("|", ",");
		m_ui->cmbCommandList->addItem(sCmd);
	
		displayMessageIN(ba.mid(16));
	}
	else if (ba.indexOf("COMMANDLISTEND") >= 0)	 //ElseIf InStr(1, szMsg, "COMMANDLISTEND") = 1 Then
		;
	else
		displayMessageIN(ba);
}

void MMainDialog::onACOMMessageSent(QByteArray ba)
{
	displayMessageOUT(ba);
}


/*-------------------------------- END OF FILE ------------------------------*/