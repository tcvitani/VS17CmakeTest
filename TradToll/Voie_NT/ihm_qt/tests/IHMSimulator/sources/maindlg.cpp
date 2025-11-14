#include <QtGui>
#include <Qlistwidget>
#include <qtconcurrentrun.h>
#include <QTextCodec>
#include <QMessageBox>

#include "constants.h"
#include "affichage.h"
#include "authorization.h"
#include "videodlg.h"
#include "config.h"
#include "touchedlg.h"
#include "saisiedlg.h"
#include "SaisieExDlg.h"
#include "policedlg.h"
#include "SimuSendReciveThread.h"
#include "maindlg.h"
#include "linkdlg.h"
#include "detectiondlg.h"
#include "AsyncInputDlg.h"
#include "ihm.h"
#include "MWaitUserDlg.h"

// macros
#define __GET_COF_ACTION			m_pSendReciveThread->getConfigAction()
#define __GET_AFF_ACTION			m_pSendReciveThread->getAffichageSetAction()
#define __GET_POL_ACTION			m_pSendReciveThread->getPoliceAction()
#define __GET_TOU_ACTION			m_pSendReciveThread->getToucheAction()
#define __GET_SAI_ACTION			m_pSendReciveThread->getSaisieAction()
#define __GET_SAI_EX_ACTION			m_pSendReciveThread->getSaisieExAction()
#define __GET_DET_ACTION			m_pSendReciveThread->getDetectionAction()
#define __GET_TEXT_ACTION			m_pSendReciveThread->getSetTextAction()
#define __GET_SCRIPT_ACTION			m_pSendReciveThread->getScriptAction()
#define __GET_VIDEO_ACTION			m_pSendReciveThread->getVideoAction()
#define __GET_AUT_ACTION			m_pSendReciveThread->getAuthAction()
#define __GET_ASY_ACTION			m_pSendReciveThread->getAsyncInputAction()

MainDlg::MainDlg(QWidget *parent, QString sIHMMailbox)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	m_bExit = false;

	m_sIHMMailbox = sIHMMailbox;
	
	QSettings regValues(QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\IHM\\%1").arg(sIHMMailbox), QSettings::NativeFormat);
	m_sSystemEncoding = regValues.value("SystemEncoding", "").toString();
	if(m_sSystemEncoding!="")
	{
		QTextCodec *codec = QTextCodec::codecForName(m_sSystemEncoding.toAscii().data());
		if(codec!=NULL)
			QTextCodec::setCodecForLocale(codec);
	}
		
		
	memset(&m_AuthData, NULL, sizeof(struct_ihm_authorization));


	//EXIT
	//QObject::connect(ui.actionExitMenu, SIGNAL(activated()), this, SLOT(onWaitExit()));
	
	//Config
	QObject::connect(ui.actionSet_Config, SIGNAL(activated()), this, SLOT(onSet_Config()));
	QObject::connect(ui.actionGet_Config, SIGNAL(activated()), this, SLOT(onGet_Config()));

	//Affichage
	QObject::connect(ui.actionDebutAff, SIGNAL(activated()), this, SLOT(onDebutAff()));
	QObject::connect(ui.actionSet_Icon, SIGNAL(activated()), this, SLOT(onSet_Icon()));
	QObject::connect(ui.actionSet_Label, SIGNAL(activated()), this, SLOT(onSet_Label()));
	QObject::connect(ui.actionSet_Image, SIGNAL(activated()), this, SLOT(onSet_Image()));
	QObject::connect(ui.actionLink, SIGNAL(activated()), this, SLOT(onLink()));
	QObject::connect(ui.actionFinAff, SIGNAL(activated()), this, SLOT(onFinAff()));

	//Touche
	QObject::connect(ui.actionDebutTouche, SIGNAL(activated()), this, SLOT(onDebutTouche()));
	QObject::connect(ui.actionDemandeTouche, SIGNAL(activated()), this, SLOT(onDemandeTouche()));
	QObject::connect(ui.actionFinTouche, SIGNAL(activated()), this, SLOT(onFinTouche()));

	//Sasie
	QObject::connect(ui.actionDemandeSasie, SIGNAL(activated()), this, SLOT(onDemandeSasie()));
	QObject::connect(ui.actionAnnulationSasie, SIGNAL(activated()), this, SLOT(onAnnulationSasie()));

	//SasieEx
	QObject::connect(ui.actionDemandeSasieEx, SIGNAL(activated()), this, SLOT(onDemandeSasieEx()));
	QObject::connect(ui.actionAnnulationSasieEx, SIGNAL(activated()), this, SLOT(onAnnulationSasieEx()));

	//Detection
	QObject::connect(ui.actionDebutDetection, SIGNAL(activated()), this, SLOT(onDebutDetection()));
	QObject::connect(ui.actionFinDetection, SIGNAL(activated()), this, SLOT(onFinDetection()));

	//Police
	QObject::connect(ui.actionSetPolice, SIGNAL(activated()), this, SLOT(onSetPolice()));

	//Video
	QObject::connect(ui.actionVideo, SIGNAL(activated()), this, SLOT(onVideo()));

	//Arret
	QObject::connect(ui.actionARRET, SIGNAL(activated()), this, SLOT(onArret()));
	
	//About dlg
	QObject::connect(ui.actionAbout_IHM_Simulator, SIGNAL(activated()), this, SLOT(onAbout_IHM_Simulator()));

	//Script
	QObject::connect(ui.actionLoad_script, SIGNAL(activated()), this, SLOT(onScript()));
	QObject::connect(ui.actionLoopScript, SIGNAL(changed()), this, SLOT(onLoopScriptChange()));
	

	
    QObject::connect(ui.btnClose, SIGNAL(clicked()), this, SLOT(onClose()));

	//Authorization
	QObject::connect(ui.actionAuthDEBUT, SIGNAL(activated()), this, SLOT(onAuthDebut()));
	QObject::connect(ui.actionAuthFIN, SIGNAL(activated()), this, SLOT(onAuthFin()));
	QObject::connect(ui.actionAUTH_AUTO, SIGNAL(activated()), this, SLOT(onAuthAutoChecked()));
	QObject::connect(ui.actionTAKE_CTRL_AUTO, SIGNAL(activated()), this, SLOT(onAuthTakeCtrlAutoChecked()));
	QObject::connect(ui.actionRETURN_CTRL_AUTO, SIGNAL(activated()), this, SLOT(onAuthReturnCtrlAutoChecked()));
	QObject::connect(ui.actionAUTH_RSP, SIGNAL(activated()), this, SLOT(onAuthRsp()));
	QObject::connect(ui.actionTAKE_CTRL_RSP, SIGNAL(activated()), this, SLOT(onAuthTakeCtrlRsp()));
	QObject::connect(ui.actionRETURN_CTRL_RSP, SIGNAL(activated()), this, SLOT(onAuthReturnCtrlRsp()));
	
	//Affichage
	QObject::connect(ui.actionDebutAsync, SIGNAL(activated()), this, SLOT(onAsyncInputDebut()));
	QObject::connect(ui.actionSetAsync, SIGNAL(activated()), this, SLOT(onAsyncInputSet()));
	QObject::connect(ui.actionGetAsync, SIGNAL(activated()), this, SLOT(onAsyncInputGet()));
	QObject::connect(ui.actionFinAsync, SIGNAL(activated()), this, SLOT(onAsyncInputFin()));


	//Send receive thread
	m_pSendReciveThread = new SimuSendReciveThread(m_sIHMMailbox);
	m_pSendReciveThread->start();
	//ReadWrite thread
	QObject::connect(__GET_TEXT_ACTION, SIGNAL(triggered()), this, SLOT(onSetText()), Qt::DirectConnection);
	QObject::connect(m_pSendReciveThread, SIGNAL(message(QString)), this, SLOT(onMessage(QString)));
	QObject::connect(m_pSendReciveThread, SIGNAL(authData(struct_ihm_authorization*)), this, SLOT(onAuthData(struct_ihm_authorization*)));
	
	QObject::connect(this, SIGNAL(prependScript(QString)), m_pSendReciveThread, SLOT(onPrependScript(QString)));
	QObject::connect(this, SIGNAL(appendScript(QString)), m_pSendReciveThread, SLOT(onAppendScript(QString)));
	QObject::connect(m_pSendReciveThread, SIGNAL(openWaitDialog()), this, SLOT(onWaitUserReq()));
	


	onAuthAutoChecked();
	onAuthTakeCtrlAutoChecked();
	onAuthReturnCtrlAutoChecked();
	ui.listWidget->setMaximumBlockCount(1000);

}

void MainDlg::onMessage(QString msg)
{
//	int iMsgEvents = m_pSendReciveThread->decMsgsEvents();
	// print message to main window
	//QString sNewLine = QString("Queued msgs: %1 - ").arg(iMsgEvents) + msg ;
	ui.listWidget->appendPlainText(msg);
}


void MainDlg::on_aboutQtButton_clicked()
{
    QApplication::aboutQt();
}

void MainDlg::onWaitExit()
{
	m_bExit = true;
	m_pSendReciveThread->getArretAction()->activate(QAction::Trigger);
}

void MainDlg::onSet_Config()
{
	m_pConfDlg = new ConfigDlg(this, ConfigDlg::CONF_SET, m_sIHMMailbox);
	
	m_pConfDlg->setAction(__GET_COF_ACTION);
	
	m_pConfDlg->exec();
	
	delete m_pConfDlg;
	m_pConfDlg = NULL;
}

void MainDlg::onGet_Config()
{
	__GET_COF_ACTION->setProperty(_PROP_TYPE, _PROP_VAL_GET);
	__GET_COF_ACTION->activate(QAction::Trigger);
}

void MainDlg::onDebutAff()
{
	__GET_AFF_ACTION->setProperty(_PROP_TYPE, _PROP_VAL_DEBUT);
	__GET_AFF_ACTION->activate(QAction::Trigger);
}

void MainDlg::onSet_Icon()
{
	m_pAffDlg = new AffichageDlg(this, AffichageDlg::AFF_ICON, m_sIHMMailbox);
	
	m_pAffDlg->setAction(__GET_AFF_ACTION);

	m_pAffDlg->exec();

	delete m_pAffDlg;
	m_pAffDlg = NULL;
}

void MainDlg::onSet_Label()
{
	m_pAffDlg = new AffichageDlg(this, AffichageDlg::AFF_LABEL, m_sIHMMailbox, m_pSendReciveThread->getLanguage());
	m_pAffDlg->setAction(__GET_AFF_ACTION);
	m_pAffDlg->exec();
	
	delete m_pAffDlg;
	m_pAffDlg = NULL;
}

void MainDlg::onSet_Image()
{
	m_pAffDlg = new AffichageDlg(this, AffichageDlg::AFF_IMAGE, m_sIHMMailbox, m_pSendReciveThread->getLanguage());
	m_pAffDlg->setAction(__GET_AFF_ACTION);
	m_pAffDlg->exec();
	
	delete m_pAffDlg;
	m_pAffDlg = NULL;
}

void MainDlg::onVideo()
{
	m_pVideoDlg = new VideoDlg(this, m_sIHMMailbox);
	m_pVideoDlg->setAction(__GET_VIDEO_ACTION);
	m_pVideoDlg->exec();

	delete m_pVideoDlg;
	m_pVideoDlg = NULL;
}

void MainDlg::onLink()
{
	m_pLinkDlg = new LinkDlg(this, m_sIHMMailbox);
	m_pLinkDlg->setAction(__GET_AFF_ACTION);
	m_pLinkDlg->exec();
	
	delete m_pLinkDlg;
	m_pLinkDlg = NULL;
}

void MainDlg::onFinAff()
{
	__GET_AFF_ACTION->setProperty(_PROP_TYPE, _PROP_VAL_FIN);
	__GET_AFF_ACTION->activate(QAction::Trigger);
}

void MainDlg::onDebutTouche()
{
	__GET_TOU_ACTION->setProperty(_PROP_TYPE, _PROP_VAL_DEBUT);
	__GET_TOU_ACTION->activate(QAction::Trigger);
}

void MainDlg::onDemandeTouche()
{
	m_pToucheDlg = new ToucheDlg(this, m_sIHMMailbox);
	
	m_pToucheDlg->setAction(__GET_TOU_ACTION);
	
	m_pToucheDlg->exec();
	
	delete m_pToucheDlg;
	m_pToucheDlg = NULL;
}

void MainDlg::onFinTouche()
{
	__GET_TOU_ACTION->setProperty(_PROP_TYPE, _PROP_VAL_FIN);
	__GET_TOU_ACTION->activate(QAction::Trigger);
}

void MainDlg::onDemandeSasie()
{
	m_pSaisieDlg = new SaisieDlg(this, m_sIHMMailbox);
	
	m_pSaisieDlg->setAction(__GET_SAI_ACTION);
	
	m_pSaisieDlg->exec();
	
	delete m_pSaisieDlg;
	m_pSaisieDlg = NULL;
}

void MainDlg::onAnnulationSasie()
{
	__GET_SAI_ACTION->setProperty(_PROP_TYPE, _PROP_VAL_ANULATE);
	__GET_SAI_ACTION->activate(QAction::Trigger);
}

void MainDlg::onDemandeSasieEx()
{
	m_pSaisieExDlg = new SaisieExDlg(this, m_sIHMMailbox);
	
	m_pSaisieExDlg->setAction(__GET_SAI_EX_ACTION);
	
	m_pSaisieExDlg->exec();
	
	delete m_pSaisieExDlg;
	m_pSaisieExDlg = NULL;
}

void MainDlg::onAnnulationSasieEx()
{
	__GET_SAI_EX_ACTION->setProperty(_PROP_TYPE, _PROP_VAL_ANULATE);
	__GET_SAI_EX_ACTION->activate(QAction::Trigger);
}

void MainDlg::onDebutDetection()
{
	m_pDetectionDlg = new DetectionDlg(this, m_sIHMMailbox);
	
	m_pDetectionDlg->setAction(__GET_DET_ACTION);
	
	m_pDetectionDlg->exec();
	
	delete m_pDetectionDlg;
	m_pDetectionDlg = NULL;
}

void MainDlg::onFinDetection()
{
	__GET_DET_ACTION->setProperty(_PROP_TYPE, _PROP_VAL_FIN);
	__GET_DET_ACTION->activate(QAction::Trigger);
}

void MainDlg::onSetPolice()
{
	m_pPoliceDlg = new PoliceDlg(this, m_sIHMMailbox);
	
	m_pPoliceDlg->setAction(__GET_POL_ACTION);
	
	m_pPoliceDlg->exec();
	
	delete m_pPoliceDlg;
	m_pPoliceDlg = NULL;
}

void MainDlg::onArret()
{
	m_pSendReciveThread->getArretAction()->activate(QAction::Trigger);
}

void MainDlg::onAbout_IHM_Simulator()
{
}

void MainDlg::onScript()
{
	QStringList slstScript = QFileDialog::getOpenFileNames(this, tr("Open script"), "", tr("XML script Files (*.xml)"));

	QString sScript;

	if(slstScript.size()>0)
	{
		bool bPrepend = true;

		if(slstScript.size()>1) 
					bPrepend = false;

		for (int i=0; i<slstScript.size(); i++)
		{
			sScript = slstScript.at(i);

			if(!sScript.isEmpty())
			{
				executeScript(sScript, bPrepend);
			}
		}

	}
}

void MainDlg::onLoopScriptChange()
{
	m_pSendReciveThread->setLoopScript(ui.actionLoopScript->isChecked());
}



void MainDlg::onScriptAction()
{
}

void MainDlg::onSetText()
{
	if(__GET_TEXT_ACTION->objectName().left(5).toInt() == ARRET)
	{
		if(m_bExit == true)
			this->close();
	}
	QString sNewLine = __GET_TEXT_ACTION->objectName().midRef(5).toString();
	onMessage(sNewLine);
}



void MainDlg::closeEvent(QCloseEvent * event)
{
	m_bExit = true;
//	m_pSendReciveThread->getArretAction()->activate(QAction::Trigger);
//	event->ignore();
}



void MainDlg::onAuthDebut()
{
	struct_ihm_authorization newAuth;
	memset(&newAuth, 0, sizeof(newAuth));//initialize to zeroes

	m_pSendReciveThread->sendMessage((enum_ihm_service)M_IHM_AUTHORIZATION, SRV_TYP_DEBUT, &newAuth);
}

void MainDlg::onAuthFin()
{
	struct_ihm_authorization newAuth;
	memset(&newAuth, 0, sizeof(newAuth));//initialize to zeroes

	m_pSendReciveThread->sendMessage((enum_ihm_service)M_IHM_AUTHORIZATION, SRV_TYP_FIN, &newAuth);
}

void MainDlg::onAuthAutoChecked()
{
	m_pSendReciveThread->m_bAuthAuto = ui.actionAUTH_AUTO->isChecked();
}

void MainDlg::onAuthTakeCtrlAutoChecked()
{
	m_pSendReciveThread->m_bTakeControlAuto = ui.actionTAKE_CTRL_AUTO->isChecked();
}

void MainDlg::onAuthReturnCtrlAutoChecked()
{
	m_pSendReciveThread->m_bReturnControlAuto = ui.actionRETURN_CTRL_AUTO->isChecked();
}

void MainDlg::onAuthRsp()
{
	m_pAuthDlg = new AuthorizationDlg(this, IHM_AUTH_RSP, m_sIHMMailbox);
	
	m_pAuthDlg->setAction(m_pSendReciveThread->getAuthAction());
	m_pAuthDlg->setData(m_AuthData);
	
	m_pAuthDlg->exec();
	
	delete m_pAuthDlg;
	m_pAuthDlg = NULL;
}

void MainDlg::onAuthTakeCtrlRsp()
{
	m_pAuthDlg = new AuthorizationDlg(this, IHM_TAKE_CTRL_RSP, m_sIHMMailbox);
	
	m_pAuthDlg->setAction(__GET_AUT_ACTION);
	m_pAuthDlg->setData(m_AuthData);
	
	m_pAuthDlg->exec();
	
	delete m_pAuthDlg;
	m_pAuthDlg = NULL;
}

void MainDlg::onAuthReturnCtrlRsp()
{
	m_pAuthDlg = new AuthorizationDlg(this, IHM_RETURN_CTRL_RSP, m_sIHMMailbox);
	
	m_pAuthDlg->setAction(__GET_AUT_ACTION);
	m_pAuthDlg->setData(m_AuthData);
	
	m_pAuthDlg->exec();
	
	delete m_pAuthDlg;
	m_pAuthDlg = NULL;
}


void MainDlg::onAuthData(struct_ihm_authorization *newData)
{
	memcpy(&m_AuthData, newData, sizeof(struct_ihm_authorization));
	delete newData;
}


void MainDlg::onClose()
{
	ui.actionARRET->trigger();
	close();
}	

void MainDlg::executeScript(QString szScript, bool bPrepend)
{
	if(!szScript.isNull())
	{
		QString sFullPath;
		QString sFileName;
		QString sRelativeDir;
		
		int iSlash = szScript.lastIndexOf(QRegExp("\\\\|/"));
		QDir oDir;

		if(iSlash>=0)
		{
			sRelativeDir = szScript.left(iSlash);
			sFileName = szScript.mid(iSlash+1);
			oDir.setPath(sRelativeDir);
		}
			
		sFullPath = oDir.absoluteFilePath(sFileName);
		sFullPath = QDir::cleanPath(sFullPath);

		if(bPrepend)
			emit prependScript(sFullPath);
		else
			emit appendScript(sFullPath);

	}
	
}

void MainDlg::onAsyncInputDebut()
{
	struct_ihm_async_input newAsyncInput = {0};
	
	m_pSendReciveThread->sendMessage((enum_ihm_service)M_IHM_ASYNC_INPUT, SRV_TYP_DEBUT, &newAsyncInput);
}

void MainDlg::onAsyncInputSet()
{
	m_pAsyncInputDlg = new AsyncInputDlg(this, m_sIHMMailbox);
	
	m_pAsyncInputDlg->setAction(__GET_ASY_ACTION);
	
	m_pAsyncInputDlg->exec();
	
	delete m_pAsyncInputDlg;
	m_pAsyncInputDlg = NULL;
}

void MainDlg::onAsyncInputGet()
{
	struct_ihm_async_input newAsyncInput = {0};
	
	m_pSendReciveThread->sendMessage((enum_ihm_service)M_IHM_ASYNC_INPUT, SRV_TYP_GET, &newAsyncInput);
}


void MainDlg::onAsyncInputFin()
{
	struct_ihm_async_input newAsyncInput = {0};
	
	m_pSendReciveThread->sendMessage((enum_ihm_service)M_IHM_ASYNC_INPUT, SRV_TYP_FIN, &newAsyncInput);
}


void MainDlg::onWaitUserReq()
{
	
	MWaitUserDlg *pDlg = new MWaitUserDlg(this);

	bool bCancelScript = (QDialog::Rejected == pDlg->exec());

	delete pDlg;
	pDlg = NULL;

	m_pSendReciveThread->setUserDialogConfirmed(bCancelScript);

}