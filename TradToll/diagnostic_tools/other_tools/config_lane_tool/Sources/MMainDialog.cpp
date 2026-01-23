/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 congif_lane_tool												 */
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

#include <QtGui>
#include <QFileDialog>
#include <QFileInfo>
#include <QUrl>
#include <QThread>
#include <QDesktopServices>
#include <QProcess>
#include <QRegularExpressionValidator>
#include <QLineEdit>
#include <QMessageBox>

#include "ui_MMainDialogB.h"


#include "MDefines.h"
#include "MGlobalStructs.h"
#include "MMainDialog.h"
#include "MTranslator.h"
#include "MCfgToolConfig.h"
#include "MAboutWidget.h"
#include "MStyledToolBox.h"
#include "MStyledLangPick.h"
#include "MStyledTitleBar.h"
#include "MLangPickConfig.h"
#include "MTracer.h"
#include "MRegUtils.h"

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

MMainDialog::MMainDialog(QWidget *parent) :m_ui(new Ui::MMainDialogB), m_pAboutDlg(NULL), m_pMainToolBox(NULL), QDialog(parent)
{
	m_ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);

	//setAttribute(Qt::WA_TranslucentBackground);
    //setStyleSheet("#MMainDialog{ background:transparent;opacity:0.3;}");
	
	setWindowIcon(QIcon(MConfig::getCfg()->getWindowIconPath()));

	QString sVer = MHelpFuncs::getVersionInfo();
	
	m_sTitle = QString(CFG_LANE_TOOL_LABEL_ERR_TITLE) + QString(" v.%1").arg(sVer);
	setWindowTitle(m_sTitle);

	//Init window styled controls...
	//--Title bar
	m_pTitle = new MStyledTitleBar(this, m_ui->frTitle, m_ui->btnMinimize, m_ui->btnRestoreMax, m_ui->btnClose);
	m_pTitle->setButtonsPixmaps(QPixmap(QString::fromUtf8(":/Images/maximize.png")),
								QPixmap(QString::fromUtf8(":/Images/minimize.png")),
								QPixmap(QString::fromUtf8(":/Images/restore.png")),
								QPixmap() );

	connect(m_pTitle, SIGNAL(dialogResized()), this, SLOT(onDialogResized()));
	//to enable for debuging...
	//connect(m_pTitle, SIGNAL(message(QString)), this, SLOT(onMessage(QString)));

	//--Options menu..
	createMainToolBox();
	connect(m_ui->btnTools, SIGNAL(released()), this, SLOT(onBtnTools()));

    //--Languages menu...
	createLangPick();
	m_LangPick->setSelectedLang(MConfig::getCfg()->getDefaultLanguageId());
	
	//--About window ...
	createAboutDlg();
	connect(m_ui->btnAbout, SIGNAL(released()), this, SLOT(onBtnAbout()));
	
	// plaza number - spinbox
	connect(m_ui->spinBoxPlazaNumber, SIGNAL(valueChanged(int)), this, SLOT(onPlazaNumChanged(int)));

	// lane number - spinbox
	connect(m_ui->spinBoxLaneNumber, SIGNAL(valueChanged(int)), this, SLOT(onLaneNumChanged(int)));

	// quit button
	connect(m_ui->btnQuit, SIGNAL(released()), this, SLOT(onBtnQuit()));

	// save button
	connect(m_ui->btnSave, SIGNAL(released()), this, SLOT(onBtnSave()));

	// button "general tab - get actual configuration"
	connect(m_ui->btnGeneralGetCurrent, SIGNAL(released()), this, SLOT(onBtnGeneralGetCurrent()));

	// button "general tab - get default configuration"
	connect(m_ui->btnGeneralGetDefault, SIGNAL(released()), this, SLOT(onBtnGeneralGetDefault()));

	// button "netwotk tab - get actual configuration"
	connect(m_ui->btnNetGetCurrent, SIGNAL(released()), this, SLOT(onBtnNetworkGetCurrent()));

	// button "netwotk tab - get default configuration"
	connect(m_ui->btnNetGetDefault, SIGNAL(released()), this, SLOT(onBtnNetworkGetDefault()));

	// comboBoxNetworkCard 
	connect(m_ui->comboBoxNetworkCard, SIGNAL(activated(QString)), this, SLOT(onComboBoxNetCard(QString)));

	// checkBoxAutoIp
	connect(m_ui->checkBoxAutoIp, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxAutoIp(bool)));

	// checkBoxUpdateIp
	connect(m_ui->checkBoxUpdateIp, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxUpdateIp(bool)));

	// button "devices tab - get actual configuration"
	connect(m_ui->btnDevGetCurrent, SIGNAL(released()), this, SLOT(onBtnDevGetCurrent()));

	// button "devices tab - get default configuration"
	connect(m_ui->btnDevGetDefault, SIGNAL(released()), this, SLOT(onBtnDevGetDefault()));

	// button "plaza specifica tab - get actual configuration"
	connect(m_ui->btnPlazaGetCurrent, SIGNAL(released()), this, SLOT(onBtnPlazaGetCurrent()));

	// button "plaza specifica tab - get default configuration"
	connect(m_ui->btnPlazaGetDefault, SIGNAL(released()), this, SLOT(onBtnPlazaGetDefault()));

	// button "lane specifica tab - get actual configuration"
	connect(m_ui->btnLaneGetCurrent, SIGNAL(released()), this, SLOT(onBtnLaneGetCurrent()));

	// button "lane specifica tab - get default configuration"
	connect(m_ui->btnLaneGetDefault, SIGNAL(released()), this, SLOT(onBtnLaneGetDefault()));

	// Load stylesheet file if path to that file isn't empty and if that file isn't empty.
	if (!MCfgLaneToolConfig::getCfg()->getStylesheetPath().isEmpty() && !MHelpFuncs::loadFileContent(MCfgLaneToolConfig::getCfg()->getStylesheetPath()).isEmpty())
		MHelpFuncs::setFileCSSToWidget(MCfgLaneToolConfig::getCfg()->getStylesheetPath(), this);

	// init configuration
	initConfiguration();

	// Disable autoDefault property on all buttons
	foreach (QPushButton *button, this->findChildren<QPushButton*>())
		button->setAutoDefault(false);

	// Resize window if valid value of WindowSize is set in .ini file
	QString qsWndSizeTemp = MCfgLaneToolConfig::getCfg()->getWindowSize();
	if (!qsWndSizeTemp.isEmpty())
	{
		int iWidth		= 0;
		int iHeight		= 0;
		bool bWidthOk	= false;
		bool bHeightOk	= false;
		
		QStringList qsWndSizeList = qsWndSizeTemp.split("x", Qt::SkipEmptyParts, Qt::CaseInsensitive);
		
		iWidth = qsWndSizeList.at(0).toInt(&bWidthOk);
		iHeight = qsWndSizeList.at(1).toInt(&bHeightOk);
		
		if (qsWndSizeList.count() == 2 && bWidthOk && bHeightOk && iWidth>0 && iHeight>0)
			resize (iWidth, iHeight);
	}
	
	// Move window if valid value of WindowPosition is set in .ini file
	QString qsWndPosTemp = MCfgLaneToolConfig::getCfg()->getWindowPosition();
	if (!qsWndPosTemp.isEmpty())
	{
		int iX		= 0;
		int iY		= 0;
		bool bXOk	= false;
		bool bYOk	= false;
		
		QStringList qsWndPosList = qsWndPosTemp.split(".", Qt::SkipEmptyParts, Qt::CaseInsensitive);
		
		iX = qsWndPosList.at(0).toInt(&bXOk);
		iY = qsWndPosList.at(0).toInt(&bYOk);
		
		if (qsWndPosList.count() == 2 && bXOk && bYOk)
			move (iX, iY);
	}
}

MMainDialog::~MMainDialog()
{
	if (m_ui != NULL)
	{
		delete m_ui;
		m_ui = NULL;
	}
}

void MMainDialog::onClose()
{
	close();
}

//---------------------------------------------------------

void MMainDialog::updateSettingsDlgSizeAndPos(QWidget * wToPosition)
{
	QPoint ptContentPos = m_ui->frContent->pos(); //target position - (relative to parent "frBody")
	QPoint pt = m_ui->frBody->mapTo(this, ptContentPos); //top left of the target container
	
//	int iHeightBody = m_ui->frContent->height();		
	int iWidthBody = m_ui->frContent->width();		

	int iHeight = wToPosition->height();		//get the width set in designer
	QSize size = QSize(iWidthBody, iHeight);
	
	wToPosition->resize(size);
	wToPosition->move(pt);//align top left in the target container
}

//-------------------------------------------------------

void MMainDialog::createAboutDlg()
{
	m_pAboutDlg = new MAboutWidget(this);
	m_pAboutDlg->setObjectName(QString::fromUtf8("m_pAboutDlg"));
	
	m_pAboutDlg->hide();

	m_pAboutDlg->setLogoPix(MCfgLaneToolConfig::getCfg()->getLogoPixPath());
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

void MMainDialog::onBtnQuit()
{
	QString sMessage;

	if (m_bConfigSaved)
		sMessage = QString(tr(CFG_LANE_TOOL_MSG_QUIT_APP));
	else
		sMessage = QString(tr(CFG_LANE_TOOL_MSG_QUIT_APP_UNSAVED));

	if (raiseMsgBox(this,
					sMessage,
					QString(tr(CFG_LANE_TOOL_LABEL_YES)),
					QString(tr(CFG_LANE_TOOL_LABEL_NO)),
					QMessageBox::Question))
	{
		close();
	}
}

void MMainDialog::onBtnGeneralGetCurrent()
{
	if (!raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_TABS_WILL_BE_UPDATED)),
					QString(tr(CFG_LANE_TOOL_LABEL_YES)),
					QString(tr(CFG_LANE_TOOL_LABEL_NO)),
					QMessageBox::Question))
	{
		return;
	}

	if (!getGeneralConfig() || 
		!getLaneTypeConfig() || 
		!getSpecParamsConfig(m_ui->spinBoxPlazaNumber->value(), m_ui->spinBoxLaneNumber->value()))
	{
		raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_NOT_ALL_PARAM_READ)),
					QString(tr(CFG_LANE_TOOL_LABEL_OK)),
					QString(),
					QMessageBox::Warning);
	}
}

void MMainDialog::onBtnGeneralGetDefault()
{
	int iPlazaIdx = 0;
	int iLaneIdx = 0;

	if (!raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_TABS_WILL_BE_UPDATED)),
					QString(tr(CFG_LANE_TOOL_LABEL_YES)),
					QString(tr(CFG_LANE_TOOL_LABEL_NO)),
					QMessageBox::Question))
	{
		return;
	}

	if (MCfgLaneToolConfig::getCfg()->getPlazaIndex(m_ui->spinBoxPlazaNumber->value(), &iPlazaIdx) &&
		MCfgLaneToolConfig::getCfg()->getLaneIndex(iPlazaIdx, m_ui->spinBoxLaneNumber->value(), &iLaneIdx))
	{
		updateGeneralTab(iPlazaIdx, iLaneIdx);
		updateNetworkTab(iPlazaIdx, iLaneIdx);
		updateDevicesTab(iPlazaIdx, iLaneIdx);
		updatePlazaSpecParamTab(m_ui->spinBoxPlazaNumber->value());
		updateLaneSpecParamTab(m_ui->spinBoxPlazaNumber->value(), m_ui->spinBoxLaneNumber->value());
	}
	else
	{
		raiseMsgBox(this,
				QString(tr(CFG_LANE_TOOL_NO_DEFINED_CONFIG)), 
				QString(tr(CFG_LANE_TOOL_LABEL_OK)), 
				QString(), 
				QMessageBox::Warning);
	}
}

void MMainDialog::onBtnNetworkGetCurrent()
{
	if (!getNetworkConfig())
	{
		raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_NOT_ALL_PARAM_READ)),
					QString(tr(CFG_LANE_TOOL_LABEL_OK)),
					QString(),
					QMessageBox::Warning);
	}
}

void MMainDialog::onComboBoxNetCard(QString sValue)
{
	strucrt_network_cfg	*pNetCfgInfo = NULL;

	pNetCfgInfo = getNetCfgToApplyInfo(sValue);

	updateAndShowIpAddressData(pNetCfgInfo);
}

void MMainDialog::onCheckBoxAutoIp(bool bChecked)
{
	strucrt_network_cfg	*pNetCfgInfo = NULL;

	pNetCfgInfo = getNetCfgToApplyInfo(m_ui->comboBoxNetworkCard->currentText());

	pNetCfgInfo->bAutoIpAddress = bChecked;

	if (pNetCfgInfo->bAutoIpAddress)
	{
		pNetCfgInfo->sIpAddress = QString();
		pNetCfgInfo->sSubnetMask = QString();
		pNetCfgInfo->sDefaultGateWay = QString();

		updaterNetworkTabIpAddress(true);
	}
	else
		updaterNetworkTabIpAddress(false);
}

void MMainDialog::onCheckBoxUpdateIp(bool bChecked)
{
	strucrt_network_cfg	*pNetCfgInfo = NULL;

	pNetCfgInfo = getNetCfgToApplyInfo(m_ui->comboBoxNetworkCard->currentText());

	if (pNetCfgInfo != NULL)
		pNetCfgInfo->bUpdateIpSettings = bChecked;
}

void MMainDialog::onBtnNetworkGetDefault()
{
	int iPlazaIdx = 0;
	int iLaneIdx = 0;

	if (MCfgLaneToolConfig::getCfg()->getPlazaIndex(m_ui->spinBoxPlazaNumber->value(), &iPlazaIdx) &&
		MCfgLaneToolConfig::getCfg()->getLaneIndex(iPlazaIdx, m_ui->spinBoxLaneNumber->value(), &iLaneIdx))
	{
		updateNetworkTab(iPlazaIdx, iLaneIdx);
	}
	else
	{
		raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_NO_DEFINED_CONFIG)),
					QString(tr(CFG_LANE_TOOL_LABEL_OK)),
					QString(),
					QMessageBox::Warning);
	}
}

void MMainDialog::onBtnDevGetCurrent()
{
	if (!getLaneTypeConfig())
	{
		raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_NOT_ALL_PARAM_READ)),
					QString(tr(CFG_LANE_TOOL_LABEL_OK)),
					QString(),
					QMessageBox::Warning);
	}
}

void MMainDialog::onBtnDevGetDefault()
{
	int iPlazaIdx = 0;
	int iLaneIdx = 0;

	if (MCfgLaneToolConfig::getCfg()->getPlazaIndex(m_ui->spinBoxPlazaNumber->value(), &iPlazaIdx) &&
		MCfgLaneToolConfig::getCfg()->getLaneIndex(iPlazaIdx, m_ui->spinBoxLaneNumber->value(), &iLaneIdx))
	{
		updateDevicesTab(iPlazaIdx, iLaneIdx);
	}
	else
	{
		raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_NO_DEFINED_CONFIG)),
					QString(tr(CFG_LANE_TOOL_LABEL_OK)),
					QString(),
					QMessageBox::Warning);
	}
}

void MMainDialog::onBtnPlazaGetCurrent()
{
	uint	uiPlazaNumber	= 0;
	bool	bResult			= false;

	// read plaza number
	uiPlazaNumber = getCurrentPlazaNumber(m_szGeneralAppParamReg, &bResult);
	if (bResult)
	{
		if (!getPlazaSpecParamsConfig(uiPlazaNumber))
		{
			raiseMsgBox(this,
						QString(tr(CFG_LANE_TOOL_NOT_ALL_PARAM_READ)),
						QString(tr(CFG_LANE_TOOL_LABEL_OK)),
						QString(),
						QMessageBox::Warning);
		}
	}
	else
	{
		raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_PLAZA_NUM_NOT_FOUND)),
					QString(tr(CFG_LANE_TOOL_LABEL_OK)),
					QString(),
					QMessageBox::Warning);
	}
}

void MMainDialog::onBtnPlazaGetDefault()
{
	updatePlazaSpecParamTab(m_ui->spinBoxPlazaNumber->value());
}

void MMainDialog::onBtnLaneGetCurrent()
{
	uint	uiPlazaNumber	= 0;
	uint	uiLaneNumber	= 0;
	bool	bResult			= false;

	// read plaza number
	uiPlazaNumber = getCurrentPlazaNumber(m_szGeneralAppParamReg, &bResult);
	if (!bResult)
	{
		raiseMsgBox(this,
			QString(tr(CFG_LANE_TOOL_PLAZA_NUM_NOT_FOUND)),
			QString(tr(CFG_LANE_TOOL_LABEL_OK)),
			QString(),
			QMessageBox::Warning);

		return;
	}

	// read lane number
	uiLaneNumber = getCurrentLaneNumber(m_szGeneralAppParamReg, &bResult);
	if (!bResult)
	{
		raiseMsgBox(this,
			QString(tr(CFG_LANE_TOOL_LANE_NUM_NOT_FOUND)),
			QString(tr(CFG_LANE_TOOL_LABEL_OK)),
			QString(),
			QMessageBox::Warning);

		return;
	}

	if (!getLaneSpecParamsConfig(uiPlazaNumber, uiLaneNumber))
	{
		raiseMsgBox(this,
			QString(tr(CFG_LANE_TOOL_NOT_ALL_PARAM_READ)),
			QString(tr(CFG_LANE_TOOL_LABEL_OK)),
			QString(),
			QMessageBox::Warning);
	}
}

void MMainDialog::onBtnLaneGetDefault()
{
	updateLaneSpecParamTab(m_ui->spinBoxPlazaNumber->value(), m_ui->spinBoxLaneNumber->value());
}

void MMainDialog::onBtnSave()
{
	if (raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_MSG_SAVE)),
					QString(tr(CFG_LANE_TOOL_LABEL_YES)),
					QString(tr(CFG_LANE_TOOL_LABEL_NO)),
					QMessageBox::Question))
	{
		TRACE_W(QString("MMainDialog::on button Save"));
		saveConfiguration();
	}		
}

void MMainDialog::saveConfiguration()

{
	TRACE_W(QString("MMainDialog::saveConfiguration()"));
	// save configuration
	if (!saveGeneralConfig() || !saveLaneTypeConfig() || !saveSpecParamsConfig() || !saveNetworkConfig())
	{
		raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_NOT_ALL_PARAM_SAVED)),
					QString(tr(CFG_LANE_TOOL_LABEL_OK)),
					QString(),
					QMessageBox::Warning);

		TRACE_W(QString("MMainDialog::saveConfiguration() => saveGeneralConfig(),saveLaneTypeConfig(),  saveSpecParamsConfig(), saveNetworkConfig()"));
	}

	// delete context file in order to make a cold start at lane application init 
	if (m_bMakeColdStart && fileExists(MCfgLaneToolConfig::getCfg()->getContextFilePath()))
	{
		if (!QFile::remove(MCfgLaneToolConfig::getCfg()->getContextFilePath()))
		{
			TRACE_W(QString("MMainDialog::saveConfiguration() => Error deleting context file: %1")
				.arg(MCfgLaneToolConfig::getCfg()->getContextFilePath()));
		}

		TRACE_W(QString("MMainDialog::saveConfiguration() => deleting context file : %1")
			.arg(MCfgLaneToolConfig::getCfg()->getContextFilePath()));
	}

	m_bConfigSaved = true;

	if (MCfgLaneToolConfig::getCfg()->forcePcShutDown())
	{
		raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_MSG_FORCE_SHUTDOWN)),
					QString(tr(CFG_LANE_TOOL_LABEL_OK)),
					QString(),
					QMessageBox::Warning);

		QProcess::startDetached(MCfgLaneToolConfig::getCfg()->getShutDownCommand());
		TRACE_W(QString("MMainDialog::saveConfiguration() => force PC shutdown"));
	}
	else if (MCfgLaneToolConfig::getCfg()->shutDownPcMessage())
	{
		if (raiseMsgBox(this,
						QString(tr(CFG_LANE_TOOL_MSG_QUESTION_SHUTDOWN)),
						QString(tr(CFG_LANE_TOOL_LABEL_OK)),
						QString(tr(CFG_LANE_TOOL_LABEL_CANCEL)),
						QMessageBox::Question))
		{
			QProcess::startDetached(MCfgLaneToolConfig::getCfg()->getShutDownCommand());
			TRACE_W(QString("MMainDialog::saveConfiguration() => PC shutdown message"));
		}
		TRACE_W(QString("MMainDialog::saveConfiguration() => get PC shutdown command"));
	}
	else
	{
		raiseMsgBox(this,
					QString(tr(CFG_LANE_TOOL_MSG_CHANGES_SAVED)),
					QString(tr(CFG_LANE_TOOL_LABEL_OK)),
					QString(),
					QMessageBox::Information);
		TRACE_W(QString("MMainDialog::saveConfiguration() => changes saved message"));
	}
}

bool MMainDialog::getGeneralConfig()
{
	uint	uiValue				= 0;
	int		iStatus				= 0;
	int		iIdx				= 0;
	bool	bNotUpdatetd		= false;
	bool	bResult				= false;
	QString	sValue;

	m_bReadActualCfg = true;

	if (!MCfgLaneToolConfig::getCfg()->doNotShowGeneralTab())
	{
		// read plaza name
		sValue = getCurrentPlazaName(m_szGeneralAppParamReg, &bResult);
		m_ui->lineEditPlazaName->setText(sValue);
		if (!bResult)
			bNotUpdatetd = true;

		// read plaza number
		uiValue = getCurrentPlazaNumber(m_szGeneralAppParamReg, &bResult);
		if (!bResult)
			bNotUpdatetd = true;
		else
		{
			if ((int)uiValue < MCfgLaneToolConfig::getCfg()->getMinPlazaNum() || (int)uiValue > MCfgLaneToolConfig::getCfg()->getMaxPlazaNum())
			{
				TRACE_W(QString("MMainDialog::getGeneralConfig() => Plaza number: %1 out of range -> min: %2, max: %3")
					.arg(uiValue)
					.arg(MCfgLaneToolConfig::getCfg()->getMinPlazaNum())
					.arg(MCfgLaneToolConfig::getCfg()->getMaxPlazaNum()));
			}
			else
				m_ui->spinBoxPlazaNumber->setValue(uiValue);
		}

		// read lane name
		sValue = MRegUtils::readStringKey(m_szGeneralAppParamReg,
										MCfgLaneToolConfig::getCfg()->getRegValLaneName().toLatin1().data(),
										&iStatus);

		m_ui->lineEditLaneName->setText(sValue);
		if (sValue.isEmpty() || iStatus != MRegUtils::SUCCESS)
		{
			bNotUpdatetd = true;

			TRACE_W(QString("MMainDialog::getGeneralConfig() => MRegUtils::readStringKey(): Error read registry %1\\%2")
				.arg(m_szGeneralAppParamReg)
				.arg(MCfgLaneToolConfig::getCfg()->getRegValLaneName().toLatin1().data()));
		}

		// read lane number
		uiValue = getCurrentLaneNumber(m_szGeneralAppParamReg, &bResult);
		if (!bResult)
			bNotUpdatetd = true;
		else
		{
			if ((int)uiValue < MCfgLaneToolConfig::getCfg()->getMinLaneNum() || (int)uiValue > MCfgLaneToolConfig::getCfg()->getMaxLaneNum())
			{
				TRACE_W(QString("MMainDialog::getGeneralConfig() => Lane number: %1 out of range -> min: %2, max: %3")
					.arg(uiValue)
					.arg(MCfgLaneToolConfig::getCfg()->getMinLaneNum())
					.arg(MCfgLaneToolConfig::getCfg()->getMaxLaneNum()));
			}
			else
				m_ui->spinBoxLaneNumber->setValue(uiValue);

		}

		// dual lane
		if (!MCfgLaneToolConfig::getCfg()->doNotShowDualLane())
		{
			uiValue = MRegUtils::readDwordKey(m_szGeneralAppParamReg,
											MCfgLaneToolConfig::getCfg()->getRegValDualLane().toLatin1().data(),
											&iStatus);

			if (iStatus != MRegUtils::SUCCESS)
			{
				bNotUpdatetd = true;

				TRACE_W(QString("MMainDialog::getGeneralConfig() => MRegUtils::readDwordKey(): Error read registry %1\\%2")
					.arg(m_szGeneralAppParamReg)
					.arg(MCfgLaneToolConfig::getCfg()->getRegValDualLane().toLatin1().data()));
			}
			else
			{
				(uiValue == 0) ? m_ui->checkBoxDualLane->setChecked(false) : m_ui->checkBoxDualLane->setChecked(true);
			}
		}

		// update lane direction
		if (!MCfgLaneToolConfig::getCfg()->doNotShowLaneDirection())
		{
			// read lane direction
			sValue = MRegUtils::readStringKey(m_szGeneralAppParamReg,
											MCfgLaneToolConfig::getCfg()->getRegValLaneDirection().toLatin1().data(),
											&iStatus);

			if (sValue.isEmpty() || iStatus != MRegUtils::SUCCESS)
			{
				bNotUpdatetd = true;

				TRACE_W(QString("MMainDialog::getGeneralConfig() => MRegUtils::readStringKey(): Error read registry %1\\%2")
					.arg(m_szGeneralAppParamReg)
					.arg(MCfgLaneToolConfig::getCfg()->getRegValLaneDirection().toLatin1().data()));
			}
			else
			{
				bNotUpdatetd = true;

				for (iIdx = 0; iIdx < m_ui->comboBoxLaneDirection->count(); iIdx++)
				{
					if (m_ui->comboBoxLaneDirection->currentText() == sValue)
					{
						m_ui->comboBoxLaneDirection->setCurrentIndex(iIdx);
						bNotUpdatetd = false;
						break;
					}
				}

				if (bNotUpdatetd)
				{
					TRACE_W(QString("MMainDialog::getGeneralConfig() => Wrong lane direction: %1")
						.arg(m_szGeneralAppParamReg)
						.arg(sValue));
				}
			}
		}

		// update fare point
		if (!MCfgLaneToolConfig::getCfg()->doNotShowFarePoint())
		{
			uiValue = MRegUtils::readDwordKey(m_szTftParamReg,
											MCfgLaneToolConfig::getCfg()->getRegValFarePoint().toLatin1().data(),
											&iStatus);

			if (iStatus != MRegUtils::SUCCESS)
			{
				bNotUpdatetd = true;

				TRACE_W(QString("MMainDialog::getGeneralConfig() => MRegUtils::readDwordKey(): Error read registry %1\\%2")
					.arg(m_szTftParamReg)
					.arg(MCfgLaneToolConfig::getCfg()->getRegValFarePoint().toLatin1().data()));
			}
			else
				m_ui->lineEditFarePoint->setText(QString("%1").arg(uiValue));
		}
	}

	m_bReadActualCfg = false;

	return !bNotUpdatetd;
}

bool MMainDialog::saveGeneralConfig()
{
	DWORD		dwValue = 0;
	bool		bNotUpdatetd = false;
	QString		sValue;
	strucrt_manual_cfg		*pManualConfig = NULL;
	int	iPlazaIdx = 0;
	int	iLaneIdx = 0;


	if (MCfgLaneToolConfig::getCfg()->doNotShowGeneralTab())
		return true;

	TRACE_W(QString("MMainDialog::saveGeneralConfig() => START"));
	// update plaza name
	sValue = m_ui->lineEditPlazaName->text();
	if (!sValue.isEmpty())
	{
		if (MRegUtils::updateStringKey(m_szGeneralAppParamReg,
									MCfgLaneToolConfig::getCfg()->getRegValPlazaName().toLatin1().data(),
									sValue.toLatin1().data()) != MRegUtils::SUCCESS)
		{
			bNotUpdatetd = true;

			TRACE_W(QString("MMainDialog::saveGeneralConfig() => MRegUtils::updateStringKey(): Error update registry %1\\%2")
				.arg(m_szGeneralAppParamReg)
				.arg(MCfgLaneToolConfig::getCfg()->getRegValPlazaName().toLatin1().data()));
		}
	}
	else
	{
		bNotUpdatetd = true;
		TRACE_W(QString("MMainDialog::saveGeneralConfig() => [m_ui->lineEditPlazaName->text() == empty]"));
	}

	// update plaza number
	dwValue = m_ui->spinBoxPlazaNumber->value();
	if (dwValue > 0)
	{
		if (MRegUtils::updateDwordKey(m_szGeneralAppParamReg,
									MCfgLaneToolConfig::getCfg()->getRegValPlazaNumber().toLatin1().data(),
									dwValue) != MRegUtils::SUCCESS)
		{
			bNotUpdatetd = true;

			TRACE_W(QString("MMainDialog::saveGeneralConfig() => MRegUtils::updateDwordKey(): Error update registry %1\\%2")
				.arg(m_szGeneralAppParamReg)
				.arg(MCfgLaneToolConfig::getCfg()->getRegValPlazaNumber().toLatin1().data()));
		}
	}
	else
	{
		bNotUpdatetd = true;
		TRACE_W(QString("MMainDialog::saveGeneralConfig() => [m_ui->spinBoxPlazaNumber->value() < 0]"));
	}

	// update lane name
	sValue = m_ui->lineEditLaneName->text();
	if (!sValue.isEmpty())
	{
		if (MRegUtils::updateStringKey(m_szGeneralAppParamReg,
									MCfgLaneToolConfig::getCfg()->getRegValLaneName().toLatin1().data(),
									sValue.toLatin1().data()) != MRegUtils::SUCCESS)
		{
			bNotUpdatetd = true;

			TRACE_W(QString("MMainDialog::saveGeneralConfig() => MRegUtils::updateStringKey(): Error update registry %1\\%2")
				.arg(m_szGeneralAppParamReg)
				.arg(MCfgLaneToolConfig::getCfg()->getRegValLaneName().toLatin1().data()));
		}
	}
	else
	{
		bNotUpdatetd = true;
		TRACE_W(QString("MMainDialog::saveGeneralConfig() => [m_ui->lineEditLaneName->text() == empty]"));
	}

	// update lane number
	dwValue = m_ui->spinBoxLaneNumber->value();
	if (dwValue > 0)
	{
		if (MRegUtils::updateDwordKey(m_szGeneralAppParamReg,
									MCfgLaneToolConfig::getCfg()->getRegValLaneNumber().toLatin1().data(),
									dwValue) != MRegUtils::SUCCESS)
		{
			bNotUpdatetd = true;

			TRACE_W(QString("MMainDialog::saveGeneralConfig() => MRegUtils::updateDwordKey(): Error update registry %1\\%2")
				.arg(m_szGeneralAppParamReg)
				.arg(MCfgLaneToolConfig::getCfg()->getRegValLaneNumber().toLatin1().data()));
		}
	}
	else
	{
		bNotUpdatetd = true;
		TRACE_W(QString("MMainDialog::saveGeneralConfig() => [m_ui->spinBoxLaneNumber->value() < 0]"));
	}

	// dual lane
	if (!MCfgLaneToolConfig::getCfg()->doNotShowDualLane())
	{
		dwValue = (m_ui->checkBoxDualLane->isChecked())?1:0;
		
		if (MRegUtils::updateDwordKey(m_szGeneralAppParamReg,
									MCfgLaneToolConfig::getCfg()->getRegValDualLane().toLatin1().data(),
									dwValue) != MRegUtils::SUCCESS)
		{
			bNotUpdatetd = true;

			TRACE_W(QString("MMainDialog::saveGeneralConfig() => MRegUtils::updateDwordKey(): Error update registry %1\\%2")
				.arg(m_szGeneralAppParamReg)
				.arg(MCfgLaneToolConfig::getCfg()->getRegValDualLane().toLatin1().data()));
		}
	}

	// update lane direction
	if (!MCfgLaneToolConfig::getCfg()->doNotShowLaneDirection())
	{
		sValue = m_ui->comboBoxLaneDirection->currentText();
		if (!sValue.isEmpty())
		{
			if (MRegUtils::updateStringKey(m_szGeneralAppParamReg,
										MCfgLaneToolConfig::getCfg()->getRegValLaneDirection().toLatin1().data(),
										sValue.toLatin1().data()) != MRegUtils::SUCCESS)
			{
				bNotUpdatetd = true;

				TRACE_W(QString("MMainDialog::saveGeneralConfig() => MRegUtils::updateStringKey(): Error update registry %1\\%2")
					.arg(m_szGeneralAppParamReg)
					.arg(MCfgLaneToolConfig::getCfg()->getRegValLaneDirection().toLatin1().data()));
			}
		}
		else
		{
			bNotUpdatetd = true;
			TRACE_W(QString("MMainDialog::saveGeneralConfig() => [m_ui->comboBoxLaneDirection->currentText() == empty]"));
		}
	}

	// update fare point
	if (!MCfgLaneToolConfig::getCfg()->doNotShowFarePoint())
	{
		dwValue = m_ui->lineEditFarePoint->text().toULong();
		if (dwValue > 0)
		{
			if (MRegUtils::updateDwordKey(m_szTftParamReg,
										MCfgLaneToolConfig::getCfg()->getRegValFarePoint().toLatin1().data(),
										dwValue) != MRegUtils::SUCCESS)
			{
				bNotUpdatetd = true;

				TRACE_W(QString("MMainDialog::saveGeneralConfig() => MRegUtils::updateDwordKey(): Error update registry %1\\%2")
					.arg(m_szTftParamReg)
					.arg(MCfgLaneToolConfig::getCfg()->getRegValFarePoint().toLatin1().data()));
			}
		}
		else
		{
			bNotUpdatetd = true;
			TRACE_W(QString("MMainDialog::saveGeneralConfig() => [m_ui->lineEditFarePoint->text().toInt() < 0]"));
		}
	}

	//update script file and execute command
	pManualConfig = MCfgLaneToolConfig::getCfg()->getManualConfig();

	MCfgLaneToolConfig::getCfg()->getPlazaIndex(m_ui->spinBoxPlazaNumber->value(), &iPlazaIdx);
	MCfgLaneToolConfig::getCfg()->getLaneIndex(iPlazaIdx, m_ui->spinBoxLaneNumber->value(), &iLaneIdx);

	if (iPlazaIdx < pManualConfig->iNbItems && iLaneIdx < pManualConfig->sPlazaConfig[iPlazaIdx].iNbItems)
	{
		ExecuteCommandRstrSettings(&pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx]);
	}

	

	return !bNotUpdatetd;
	TRACE_W(QString("MMainDialog::saveGeneralConfig() => END, %1")
		.arg(bNotUpdatetd));



}

void MMainDialog::updaterNetworkTabIpAddress(bool bSetDisabled)
{
	m_ui->lineEditIpAddress->clear();
	m_ui->lineEditSubnetMask->clear();
	m_ui->lineEditDefaultGateway->clear();

	if (bSetDisabled)
	{
		m_ui->lineEditIpAddress->setEnabled(false);
		m_ui->lineEditSubnetMask->setEnabled(false);
		m_ui->lineEditDefaultGateway->setEnabled(false);
	}
	else
	{
		m_ui->lineEditIpAddress->setEnabled(true);
		m_ui->lineEditSubnetMask->setEnabled(true);
		m_ui->lineEditDefaultGateway->setEnabled(true);
	}
}

void  MMainDialog::setNetworkTabIpAddress(QString sIpAddress, QString sIpMask, QString sDefaultGateway)
{
	m_ui->lineEditIpAddress->setEnabled(true);
	m_ui->lineEditIpAddress->setText(sIpAddress);

	m_ui->lineEditSubnetMask->setEnabled(true);
	m_ui->lineEditSubnetMask->setText(sIpMask);

	m_ui->lineEditDefaultGateway->setEnabled(true);
	m_ui->lineEditDefaultGateway->setText(sDefaultGateway);
}

strucrt_network_cfg * MMainDialog::getNetCfgToApplyInfo(QString sAdapterName)
{
	int						iIdx			= 0;
	strucrt_network_cfg		*pNetCfgInfo	= NULL;

	if (iNbNetCfg > 0)
	{
		for (iIdx = 0; iIdx < iNbNetCfg; iIdx++)
		{
			if (sNetCfgToApply[iIdx].sAdapterName == sAdapterName)
			{
				pNetCfgInfo = &sNetCfgToApply[iIdx];
				break;
			}
		}
	}

	return pNetCfgInfo;
}

void  MMainDialog::updateNetCfgToApplyWithCurrentData(QString sAdapterName)
{
	if (!m_sLastSelectedAdapterName.isEmpty())
	{
		strucrt_network_cfg	*pNetCfgInfo = NULL;

		pNetCfgInfo = getNetCfgToApplyInfo(sAdapterName);

		if (pNetCfgInfo != NULL)
		{
			pNetCfgInfo->bAutoIpAddress = m_ui->checkBoxAutoIp->isChecked();
			pNetCfgInfo->bUpdateIpSettings = m_ui->checkBoxUpdateIp->isChecked();
			pNetCfgInfo->sIpAddress = m_ui->lineEditIpAddress->text();
			pNetCfgInfo->sSubnetMask = m_ui->lineEditSubnetMask->text();
			pNetCfgInfo->sDefaultGateWay = m_ui->lineEditDefaultGateway->text();
		}
	}
}

void  MMainDialog::updateAndShowIpAddressData(strucrt_network_cfg *pNetCfg)
{
	updateNetCfgToApplyWithCurrentData(m_sLastSelectedAdapterName);

	if (pNetCfg == NULL)
	{
		m_sLastSelectedAdapterName = QString();
		return;
	}

	m_sLastSelectedAdapterName = pNetCfg->sAdapterName;

	if (pNetCfg->bAutoIpAddress)
	{
		m_ui->checkBoxAutoIp->setChecked(true);

		updaterNetworkTabIpAddress(true);
	}
	else
	{
		m_ui->checkBoxAutoIp->setChecked(false);

		setNetworkTabIpAddress(pNetCfg->sIpAddress,
							pNetCfg->sSubnetMask,
							pNetCfg->sDefaultGateWay);
	}

	updateCheckBoxUpdateIp(pNetCfg->bUpdateIpSettings);
}

void MMainDialog::updateCheckBoxUpdateIp(bool bUpdateIpSettings)
{
	if (bUpdateIpSettings)
		m_ui->checkBoxUpdateIp->setChecked(true);
	else
		m_ui->checkBoxUpdateIp->setChecked(false);
}

bool MMainDialog::getNetworkConfig()
{
	bool bRet = false;

	m_bNetTabActualCfg = true;
	m_bReadActualCfg = true;

	clearNetworkTab();

	bRet = getComputerNameAndWorkgroup();
	bRet &= getAdapterData();
	bRet &= getPlazaAndMsgServers();

	m_bReadActualCfg = false;

	return bRet;
}

bool MMainDialog::getAdapterData()
{
	bool				bFound			= false;
	ulong				uIdx			= 0;
	int					iIdx			= 0;
	int					iPlazaIdx		= 0;
	int					iLaneIdx		= 0;
	strucrt_lane_cfg	*pLaneConfig	= NULL;

	resetNetCfgToApply();
	
	if (!MCfgLaneToolConfig::getCfg()->showAllNetworkAdapters())
	{
		if (!MCfgLaneToolConfig::getCfg()->getPlazaIndex(m_ui->spinBoxPlazaNumber->value(), &iPlazaIdx) ||
			!MCfgLaneToolConfig::getCfg()->getLaneIndex(iPlazaIdx, m_ui->spinBoxLaneNumber->value(), &iLaneIdx))
			return false;

		pLaneConfig = &MCfgLaneToolConfig::getCfg()->getManualConfig()->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx];
	}

	if (ulNbAdapters > 0)
	{
		for (uIdx = 0; uIdx < ulNbAdapters; uIdx++)
		{
			// reset flag that says should ip settings be updated
			sAdaptersInfo[uIdx].bUpdateIpSettings = false;

			if (!MCfgLaneToolConfig::getCfg()->showAllNetworkAdapters())
			{
				// filter which adapters to show
				bFound = false;
				for (iIdx = 0; iIdx < pLaneConfig->iNbNetCfg; iIdx++)
				{
					if (QString(sAdaptersInfo[uIdx].FriendlyName) == pLaneConfig->sNetworkConfig[iIdx].sAdapterName)
					{
						bFound = true;
						break;
					}
				}

				if (!bFound)
					continue;
			}

			m_ui->comboBoxNetworkCard->addItem(QString(sAdaptersInfo[uIdx].FriendlyName));
			
			updateNetCfgToApplyWithActualCfg(&sAdaptersInfo[uIdx]);	
		}

		if (iNbNetCfg > 0)
		{
			m_ui->comboBoxNetworkCard->setCurrentIndex(0);

			updateAndShowIpAddressData(&sNetCfgToApply[0]);
		}
	}

	return true;
}

void MMainDialog::resetNetCfgToApply()
{
	int iIdx = 0;

	for (iIdx = 0; iIdx < MAX_NB_NETWORK_CONFIG; iIdx++)
	{
		sNetCfgToApply[iIdx].sAdapterName = QString();
		sNetCfgToApply[iIdx].bAutoIpAddress = false;
		sNetCfgToApply[iIdx].bUpdateIpSettings = false;
		sNetCfgToApply[iIdx].sIpAddress = QString();
		sNetCfgToApply[iIdx].sSubnetMask = QString();
		sNetCfgToApply[iIdx].sDefaultGateWay = QString();
	}

	iNbNetCfg = 0;

	m_sLastSelectedAdapterName = QString();
}

void MMainDialog::updateNetCfgToApplyWithActualCfg(struct_ip_adapter_info *pAdapterInfo)
{
	int iIdx = 0;
	 
	for (iIdx = 0; iIdx < iNbNetCfg; iIdx++)
	{
		if (sNetCfgToApply[iIdx].sAdapterName == QString(pAdapterInfo->FriendlyName))
			return;
	}

	sNetCfgToApply[iNbNetCfg].sAdapterName = QString(pAdapterInfo->FriendlyName);
	sNetCfgToApply[iNbNetCfg].bAutoIpAddress = pAdapterInfo->DhcpEnabled;
	sNetCfgToApply[iNbNetCfg].bUpdateIpSettings = pAdapterInfo->bUpdateIpSettings;

	if (!sNetCfgToApply[iNbNetCfg].bAutoIpAddress)
	{
		if (pAdapterInfo->dwNbIpAddresses > 0)
		{
			sNetCfgToApply[iNbNetCfg].sIpAddress = QString((char*)pAdapterInfo->sIpInfo[0].IpAddress);
			sNetCfgToApply[iNbNetCfg].sSubnetMask = QString((char*)pAdapterInfo->sIpInfo[0].IpMask);
			sNetCfgToApply[iNbNetCfg].sDefaultGateWay = QString((char*)pAdapterInfo->Gateway);
		}
		else
		{
			sNetCfgToApply[iNbNetCfg].sIpAddress = QString();
			sNetCfgToApply[iNbNetCfg].sSubnetMask = QString();
			sNetCfgToApply[iNbNetCfg].sDefaultGateWay = QString();
		}
	}

	iNbNetCfg++;
}

void MMainDialog::updateNetCfgToApplyWithDefaultCfg(int iPlazaIdx, int iLaneIdx)
{
	int					iIdx			= 0;
	strucrt_lane_cfg	*pLaneConfig	= NULL;

	pLaneConfig = &MCfgLaneToolConfig::getCfg()->getManualConfig()->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx];

	iNbNetCfg = pLaneConfig->iNbNetCfg;

	for (iIdx = 0; iIdx < pLaneConfig->iNbNetCfg; iIdx++)
	{
		sNetCfgToApply[iIdx].sAdapterName = pLaneConfig->sNetworkConfig[iIdx].sAdapterName;
		sNetCfgToApply[iIdx].bAutoIpAddress = pLaneConfig->sNetworkConfig[iIdx].bAutoIpAddress;
		sNetCfgToApply[iIdx].bUpdateIpSettings = pLaneConfig->sNetworkConfig[iIdx].bUpdateIpSettings;

		if (!sNetCfgToApply[iIdx].bAutoIpAddress)
		{
			sNetCfgToApply[iIdx].sIpAddress = pLaneConfig->sNetworkConfig[iIdx].sIpAddress;
			sNetCfgToApply[iIdx].sSubnetMask = pLaneConfig->sNetworkConfig[iIdx].sSubnetMask;
			sNetCfgToApply[iIdx].sDefaultGateWay = pLaneConfig->sNetworkConfig[iIdx].sDefaultGateWay;
		}
	}
}

bool MMainDialog::getPlazaAndMsgServers()
{
	bool	bNotUpdatetd	= false;
	int		iStatus			= 0;
	QString	sValue;

	// read file server
	sValue = MRegUtils::readStringKey(m_szLanParamReg,
									MCfgLaneToolConfig::getCfg()->getRegValFileServer().toLatin1().data(),
									&iStatus);

	if (sValue.isEmpty() || iStatus != MRegUtils::SUCCESS)
	{
		bNotUpdatetd = true;

		TRACE_W(QString("MMainDialog::getPlazaAndMsgServers() => MRegUtils::readStringKey(): Error read registry %1\\%2")
			.arg(m_szLanParamReg)
			.arg(MCfgLaneToolConfig::getCfg()->getRegValFileServer().toLatin1().data()));
	}
	m_ui->lineEditFileServer->setText(sValue);

	// read message server
	sValue = MRegUtils::readStringKey(m_szLanParamReg,
									MCfgLaneToolConfig::getCfg()->getRegValMsgServer().toLatin1().data(),
									&iStatus);

	if (sValue.isEmpty() || iStatus != MRegUtils::SUCCESS)
	{
		bNotUpdatetd = true;

		TRACE_W(QString("MMainDialog::getPlazaAndMsgServers() => MRegUtils::readStringKey(): Error read registry %1\\%2")
			.arg(m_szLanParamReg)
			.arg(MCfgLaneToolConfig::getCfg()->getRegValMsgServer().toLatin1().data()));
	}
	m_ui->lineEditMessageServer->setText(sValue);

	return !bNotUpdatetd;
}

bool MMainDialog::getComputerNameAndWorkgroup()
{
	LPWKSTA_INFO_100	pBuf			= NULL;
	bool				bRet			= false;
	QString				computerName;
	QString				workGroup;

	// Computer name and workgroup
	if (NetWkstaGetInfo(NULL, 100, (LPBYTE *)&pBuf) == NERR_Success)
	{
		// computer name
		computerName = QString::fromWCharArray(pBuf->wki100_computername);
		m_ui->lineEditComputerName->setText(computerName);

		// workgroup
		workGroup = QString::fromWCharArray(pBuf->wki100_langroup);
		m_ui->lineEditWorkGroup->setText(workGroup);

		bRet = true;
	}
	else
	{
		m_ui->lineEditComputerName->setText(QString());
		m_ui->lineEditWorkGroup->setText(QString());

		bRet = false;
	}

	if (pBuf)
	{
		NetApiBufferFree(pBuf);
		pBuf = NULL;
	}

	return bRet;
}

bool MMainDialog::saveNetworkConfig()
{
	bool				bNotUpdatetd		= false;
	char				szCommand[300]		= { 0 };
	char				szCurCompName[300]	= { 0 };
	char				szCurWorkgroup[300] = { 0 };
	int					iIdx				= 0;
	wchar_t				wcCompName[300]		= { 0 };
	LPWKSTA_INFO_100	pBuf				= NULL;
	size_t				iSize				= 0;
	QString				sComputerName;

	if (MCfgLaneToolConfig::getCfg()->doNotShowNetworkTab())
		return true;
	
	updateNetCfgToApplyWithCurrentData(m_ui->comboBoxNetworkCard->currentText());

	TRACE_W(QString("MMainDialog::saveNetworkConfig() => START"));
	for (iIdx = 0; iIdx < iNbNetCfg; iIdx++)
	{
		if (sNetCfgToApply[iIdx].bUpdateIpSettings)
		{
			// ip address
			if (sNetCfgToApply[iIdx].bAutoIpAddress)
			{
				sprintf_s(szCommand,
						sizeof(szCommand),
						MCfgLaneToolConfig::getCfg()->getSetDhcpCommand().toLatin1().data(),
						sNetCfgToApply[iIdx].sAdapterName.toLatin1().data());
			}
			else
			{
				sprintf_s(szCommand,
						sizeof(szCommand),
						MCfgLaneToolConfig::getCfg()->getSetIpAddressCommand().toLatin1().data(),
						sNetCfgToApply[iIdx].sAdapterName.toLatin1().data(),
						sNetCfgToApply[iIdx].sIpAddress.toLatin1().data(),
						sNetCfgToApply[iIdx].sSubnetMask.toLatin1().data(),
						sNetCfgToApply[iIdx].sDefaultGateWay.toLatin1().data());
			}

			// execute command to update ip address
			executeCommand(QString(szCommand));
		}
	}

	// computer name and workgroup
	if (NetWkstaGetInfo(NULL, 100, (LPBYTE *)&pBuf) == NERR_Success)
	{
		wcstombs_s(&iSize, szCurCompName, sizeof(szCurCompName), (const wchar_t*)pBuf->wki100_computername, sizeof(szCurCompName));
		wcstombs_s(&iSize, szCurWorkgroup, sizeof(szCurWorkgroup), (const wchar_t*)pBuf->wki100_langroup, sizeof(szCurWorkgroup));
	}

	if (pBuf)
		NetApiBufferFree(pBuf);

	// update workgroup
	if (QString(szCurWorkgroup) != m_ui->lineEditWorkGroup->text())
	{
		sprintf_s(szCommand,
				sizeof(szCommand),
				MCfgLaneToolConfig::getCfg()->getUpdateWorkGroupCommand().toLatin1().data(),
				szCurCompName,
				m_ui->lineEditWorkGroup->text().toLatin1().data());

		// execute command to update workgroup
		executeCommand(QString(szCommand));
	}

	// update computer name 
	sComputerName = m_ui->lineEditComputerName->text();
	if (QString(szCurCompName) != sComputerName)
	{
		sComputerName.toWCharArray(wcCompName);
		if (SetComputerName(wcCompName))
		{
			if (MRegUtils::updateStringKey(MCfgLaneToolConfig::getCfg()->getRegKeyForHostName().toLatin1().data(),
										MCfgLaneToolConfig::getCfg()->getRegValHostName().toLatin1().data(),
										sComputerName.toLatin1().data()) != MRegUtils::SUCCESS)
			{
				bNotUpdatetd = true;

				TRACE_W(QString("MMainDialog::saveNetworkConfig() => MRegUtils::updateStringKey(): Error update registry %1 %2")
					.arg(MCfgLaneToolConfig::getCfg()->getRegKeyForHostName().toLatin1().data())
					.arg(MCfgLaneToolConfig::getCfg()->getRegValHostName().toLatin1().data()));
			}

			if (MRegUtils::updateStringKey(MCfgLaneToolConfig::getCfg()->getRegKeyForHostName().toLatin1().data(),
										MCfgLaneToolConfig::getCfg()->getRegValNVHostName().toLatin1().data(),
										sComputerName.toLatin1().data()) != MRegUtils::SUCCESS)
			{
				bNotUpdatetd = true;

				TRACE_W(QString("MMainDialog::saveNetworkConfig() => MRegUtils::updateStringKey(): Error update registry %1 %2")
					.arg(MCfgLaneToolConfig::getCfg()->getRegKeyForHostName().toLatin1().data())
					.arg(MCfgLaneToolConfig::getCfg()->getRegValNVHostName().toLatin1().data()));
			}
		}
		else
		{
			bNotUpdatetd = true;

			TRACE_W(QString("MMainDialog::saveNetworkConfig() => SetComputerName(): Error set computer name: %1")
				.arg(sComputerName.toLatin1().data()));
		}
	}

	// update windows firewall
	if (!MCfgLaneToolConfig::getCfg()->getWindowsFirewallCommand().isEmpty())
	{
		strcpy_s(szCommand, 
			sizeof(szCommand),
			MCfgLaneToolConfig::getCfg()->getWindowsFirewallCommand().toLatin1().data());

		// execute command to update workgroup
		executeCommand(QString(szCommand));
	}

	// save file server
	if (MRegUtils::updateStringKey(m_szLanParamReg,
								MCfgLaneToolConfig::getCfg()->getRegValFileServer().toLatin1().data(),
								m_ui->lineEditFileServer->text().toLatin1().data()) != MRegUtils::SUCCESS)
	{
		bNotUpdatetd = true;

		TRACE_W(QString("MMainDialog::saveNetworkConfig() => MRegUtils::updateStringKey(): Error update registry %1 %2")
			.arg(m_szLanParamReg)
			.arg(m_ui->lineEditFileServer->text().toLatin1().data()));
	}

	// save message server
	if (MRegUtils::updateStringKey(m_szLanParamReg,
								MCfgLaneToolConfig::getCfg()->getRegValMsgServer().toLatin1().data(),
								m_ui->lineEditMessageServer->text().toLatin1().data()) != MRegUtils::SUCCESS)
	{
		bNotUpdatetd = true;

		TRACE_W(QString("MMainDialog::saveNetworkConfig() => MRegUtils::updateStringKey(): Error update registry %1 %2")
			.arg(m_szLanParamReg)
			.arg(m_ui->lineEditMessageServer->text().toLatin1().data()));
	}

	return !bNotUpdatetd;
	TRACE_W(QString("MMainDialog::saveNetworkConfig() => END  &1")
		.arg(bNotUpdatetd));
}

bool MMainDialog::getLaneTypeConfig()
{
	int		iStatus			= 0;
	int		iIdx			= 0;
	bool	bNotUpdatetd	= false;
	QString	sValue;
	char	szRegPath[500]	= { 0 };

	m_bReadActualCfg = true;

	if (!MCfgLaneToolConfig::getCfg()->doNotShowGeneralTab())
	{
		m_sLaneTypeForDevParam = QString();

		// read lane type
		sValue = MRegUtils::readStringKey(m_szGeneralAppParamReg,
										MCfgLaneToolConfig::getCfg()->getRegValLaneType().toLatin1().data(),
										&iStatus);

		if (sValue.isEmpty() || iStatus != MRegUtils::SUCCESS)
		{
			bNotUpdatetd = true;

			TRACE_W(QString("MMainDialog::getLaneTypeConfig() => MRegUtils::readStringKey(): Error read registry %1\\%2")
				.arg(m_szGeneralAppParamReg)
				.arg(MCfgLaneToolConfig::getCfg()->getRegValLaneType().toLatin1().data()));
		}
		else
		{
			bNotUpdatetd = true;

			for (iIdx = 0; iIdx < m_ui->comboBoxLaneType->count(); iIdx++)
			{
				if (m_ui->comboBoxLaneType->itemText(iIdx) == sValue)
				{
					m_sLaneTypeForDevParam = sValue;
					m_ui->comboBoxLaneType->setCurrentIndex(iIdx);
					bNotUpdatetd = false;
					break;
				}
			}
		}

		if (bNotUpdatetd)
		{
			TRACE_W(QString("MMainDialog::getLaneTypeConfig() => Wrong lane type: %1")
				.arg(m_szGeneralAppParamReg)
				.arg(sValue));
		}
		else if (!MCfgLaneToolConfig::getCfg()->doNotShowDevicesTab())
		{
			strucrt_lane_type	*pLaneTypesCfg = MCfgLaneToolConfig::getCfg()->getLaneTypesConfig();
			int					iIdx2 = 0;
			int					iIdx3 = 0;
			int					iKeyIdx = 0;
			DWORD				dwKeySize = 0;
			char				szKeyName[200] = { 0 };
			QString				sLaneType;

			m_ui->treeListDevices->clear();

			if (pLaneTypesCfg->iNbItems > 0)
			{
				for (iIdx = 0; iIdx < pLaneTypesCfg->iNbItems; iIdx++)
				{
					sLaneType = QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdx].sLaneType.toLatin1().data()));

					if (sLaneType == sValue && pLaneTypesCfg->sLaneTypeItem[iIdx].iNbItems > 0)
					{
						for (iIdx2 = 0; iIdx2 < pLaneTypesCfg->sLaneTypeItem[iIdx].iNbItems; iIdx2++)
						{
							QTreeWidgetItem *pTopLevel = new QTreeWidgetItem();

							pTopLevel->setText(0, QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sLabel.toLatin1().data())));

							for (iIdx3 = 0; iIdx3 < pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].iNbItems; iIdx3++)
							{
								QTreeWidgetItem *pItem = new QTreeWidgetItem();

								pItem->setText(0, QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].sLabel.toLatin1().data())));
								pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable);
								pItem->setCheckState(0, Qt::Unchecked);

								// check is device already installed
								sprintf_s(szRegPath,
									sizeof(szRegPath),
									"%s%s",
									CSR_REG_KEYn_CSRBASE,
									pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].sSubKeyPath.toLatin1().data());

								iKeyIdx = 0;  dwKeySize = sizeof(szKeyName);
								while (REG_Enum_Cles(HKEY_LOCAL_MACHINE, szRegPath, iKeyIdx, szKeyName, &dwKeySize) == ERROR_SUCCESS)
								{
									if (pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].sSubKeyName == QString(szKeyName))
									{
										pItem->setCheckState(0, Qt::Checked);
										break;
									}

									iKeyIdx++; dwKeySize = sizeof(szKeyName);
								}

								pTopLevel->addChild(pItem);
							}

							m_ui->treeListDevices->addTopLevelItem(pTopLevel);
						}

						m_ui->treeListDevices->expandAll();
						break;
					}
				}
			}
		}
	}

	m_bReadActualCfg = false;

	return !bNotUpdatetd;
}

bool MMainDialog::saveLaneTypeConfig()
{
	int					iIdx				= 0;
	int					iIdx2				= 0;
	int					iIdx3				= 0;
	int					iIdx4				= 0;
	int					iIdxLaneTypeInst	= 0;
	bool				bNotUpdatetd		= false;
	strucrt_lane_type	*pLaneTypesCfg		= NULL;
	QString				sLaneTypeDef;

	if (MCfgLaneToolConfig::getCfg()->doNotShowGeneralTab())
		return true;

	TRACE_W(QString("MMainDialog::saveLaneTypeConfig() => START"));
	if (!m_sLaneTypeForDevParam.isEmpty())
	{
		pLaneTypesCfg = MCfgLaneToolConfig::getCfg()->getLaneTypesConfig();
		
		iIdxLaneTypeInst = -1;

		if (pLaneTypesCfg->iNbItems > 0)
		{
			for (iIdx = 0; iIdx < pLaneTypesCfg->iNbItems; iIdx++)
			{
				sLaneTypeDef = QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdx].sLaneType.toLatin1().data()));
				
				if (sLaneTypeDef != m_sLaneTypeForDevParam)
				{
					// uninstall registry parameters for all other lane types
					mergeRegFiles(pLaneTypesCfg->sLaneTypeItem[iIdx].sFileDir, pLaneTypesCfg->sLaneTypeItem[iIdx].sUninstFile);

					if (!MCfgLaneToolConfig::getCfg()->doNotShowDevicesTab())
					{
						for (iIdx2 = 0; iIdx2 < pLaneTypesCfg->sLaneTypeItem[iIdx].iNbItems; iIdx2++)
						{
							for (iIdx3 = 0; iIdx3 < pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].iNbItems; iIdx3++)
							{
								// uninstall registry parameters for all devices
								mergeRegFiles(pLaneTypesCfg->sLaneTypeItem[iIdx].sFileDir, 
									pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].sUninstFile);
							}
						}
					}
				}
				else
					iIdxLaneTypeInst = iIdx;
			}
		}

		if (iIdxLaneTypeInst > -1)
		{
			// update lane type
			if (MRegUtils::updateStringKey(m_szGeneralAppParamReg,
										MCfgLaneToolConfig::getCfg()->getRegValLaneType().toLatin1().data(),
										m_sLaneTypeForDevParam.toLatin1().data()) != MRegUtils::SUCCESS)
			{
				bNotUpdatetd = true;

				TRACE_W(QString("MMainDialog::saveLaneTypeConfig() => MRegUtils::updateStringKey(): Error update registry %1\\%2")
					.arg(m_szGeneralAppParamReg)
					.arg(MCfgLaneToolConfig::getCfg()->getRegValLaneType().toLatin1().data()));
			}

			// install registry parameters for selected lane type
			mergeRegFiles(pLaneTypesCfg->sLaneTypeItem[iIdxLaneTypeInst].sFileDir, pLaneTypesCfg->sLaneTypeItem[iIdxLaneTypeInst].sInstFile);

			if (!MCfgLaneToolConfig::getCfg()->doNotShowDevicesTab())
			{
				for (iIdx = 0; iIdx < m_ui->treeListDevices->topLevelItemCount(); iIdx++)
				{
					for (iIdx3 = 0; iIdx3 < pLaneTypesCfg->sLaneTypeItem[iIdxLaneTypeInst].iNbItems; iIdx3++)
					{
						if (m_ui->treeListDevices->topLevelItem(iIdx)->text(0) ==
							QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdxLaneTypeInst].sDeviceGroup[iIdx3].sLabel.toLatin1().data())))
						{
							for (iIdx2 = 0; iIdx2 < m_ui->treeListDevices->topLevelItem(iIdx)->childCount(); iIdx2++)
							{
								for (iIdx4 = 0; iIdx4 < pLaneTypesCfg->sLaneTypeItem[iIdxLaneTypeInst].sDeviceGroup[iIdx3].iNbItems; iIdx4++)
								{
									if (m_ui->treeListDevices->topLevelItem(iIdx)->child(iIdx2)->text(0) ==
										QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdxLaneTypeInst].sDeviceGroup[iIdx3].sDeviceItem[iIdx4].sLabel.toLatin1().data())))
									{
										if (m_ui->treeListDevices->topLevelItem(iIdx)->child(iIdx2)->checkState(0) == Qt::Checked)
										{
											// install registry parameters for selected device
											mergeRegFiles(pLaneTypesCfg->sLaneTypeItem[iIdxLaneTypeInst].sFileDir,
												pLaneTypesCfg->sLaneTypeItem[iIdxLaneTypeInst].sDeviceGroup[iIdx3].sDeviceItem[iIdx4].sInstFile);
										}
										else
										{
											// uninstall registry parameters if the device doesn't selected
											mergeRegFiles(pLaneTypesCfg->sLaneTypeItem[iIdxLaneTypeInst].sFileDir,
												pLaneTypesCfg->sLaneTypeItem[iIdxLaneTypeInst].sDeviceGroup[iIdx3].sDeviceItem[iIdx4].sUninstFile);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		bNotUpdatetd = true;
		TRACE_W(QString("MMainDialog::saveLaneTypeConfig() => [m_ui->comboBoxLaneType->currentText() == empty]"));
	}

	return !bNotUpdatetd;
	TRACE_W(QString("MMainDialog::saveLaneTypeConfig() => END %1")
		.arg(bNotUpdatetd));
}

bool MMainDialog::getSpecParamsConfig(int iPlazaNum, int iLaneNum)
{
	bool bRet = false;

	bRet = getPlazaSpecParamsConfig(iPlazaNum);
	bRet &= getLaneSpecParamsConfig(iPlazaNum, iLaneNum);

	return bRet;
}

bool MMainDialog::saveSpecParamsConfig()
{
	bool bRet = false;
	TRACE_W(QString("MMainDialog::saveSpecParamsConfig() => START"));

	bRet = savePlazaSpecParamsConfig();
	bRet &= saveLaneSpecParamsConfig();

	return bRet;
	TRACE_W(QString("MMainDialog::saveSpecParamsConfig() => END  %1")
		.arg(bRet));
}

bool MMainDialog::getPlazaSpecParamsConfig(int iPlazaNum)
{
	strucrt_spec_params	*pSpecParams	= NULL;
	bool				bNotUpdatetd	= false;
	bool				bFoundPlaza		= false;
	int					iIdx			= 0;
	int					iParamIdx		= 0;
	int					iStatus			= 0;
	uint				uiParamValue	= 0;
	char				szRegPath[500]	= { 0 };
	QString				sParamValue;

	m_bReadActualCfg = true;

	if (!MCfgLaneToolConfig::getCfg()->doNotShowPlazaSpecificTab())
	{
		pSpecParams = MCfgLaneToolConfig::getCfg()->getSpecParams();
		
		m_ui->tablePlazaSpecParams->setRowCount(0);
		m_uiPlazaNumForPlazaSpecParam = 0;

		if (pSpecParams->iNbPlazaItems > 0)
		{
			for (iIdx = 0; iIdx < pSpecParams->iNbPlazaItems; iIdx++)
			{
				if (pSpecParams->sPlazaParams[iIdx].iPlazaNum == iPlazaNum)
				{
					bFoundPlaza = true;
					m_uiPlazaNumForPlazaSpecParam = iPlazaNum;

					for (iParamIdx = 0; iParamIdx < pSpecParams->sPlazaParams[iIdx].iNbItems; iParamIdx++)
					{
						// registry path
						sprintf_s(szRegPath,
								sizeof(szRegPath),
								"%s%s",
								CSR_REG_KEYn_CSRBASE,
								pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sParamRegKey.toLatin1().data());

						if (pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sParamType == QString(CFG_LANE_TOOL_PARAM_NUMBER))
						{
							// read specific parameter - number
							uiParamValue = MRegUtils::readDwordKey(szRegPath,
														pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sParamName.toLatin1().data(),
														&iStatus);

							if (iStatus != MRegUtils::SUCCESS)
							{
								sParamValue = pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sDefaultValue;

								bNotUpdatetd = true;

								TRACE_W(QString("MMainDialog::getPlazaSpecParamsConfig() => MRegUtils::readDwordKey(): Error read registry %1\\%2")
									.arg(szRegPath)
									.arg(pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sParamName.toLatin1().data()));
							}
							else
								sParamValue = QString("%1").arg(uiParamValue);
						}
						else
						{
							// read specific parameter - string
							sParamValue = MRegUtils::readStringKey(szRegPath,
														pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sParamName.toLatin1().data(),
														&iStatus);

							if (iStatus != MRegUtils::SUCCESS)
							{
								sParamValue = pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sDefaultValue;

								bNotUpdatetd = true;

								TRACE_W(QString("MMainDialog::getPlazaSpecParamsConfig() => MRegUtils::readStringKey(): Error read registry %1\\%2")
									.arg(szRegPath)
									.arg(pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sParamName.toLatin1().data()));
							}
						}

						addTableRow(m_ui->tablePlazaSpecParams,
							QString(tr(pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sParamLabel.toLatin1().data())),
							sParamValue,
							pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sParamType);
					}

					break;
				}
			}
		}
	}

	if (!bFoundPlaza)
	{
		bNotUpdatetd = true;

		TRACE_W(QString("MMainDialog::getPlazaSpecParamsConfig() => Not found defined specific parameters for plaza number: %1")
			.arg(iPlazaNum));
	}

	m_bReadActualCfg = false;

	return !bNotUpdatetd;
}

bool MMainDialog::savePlazaSpecParamsConfig()
{
	strucrt_spec_params	*pSpecParams		= NULL;
	int					iIdx				= 0;
	int					iIdx2				= 0;
	int					iSelectedPlaza		= 0;
	char				szRegPath[MAX_PATH] = { 0 };
	bool				bNotUpdatetd		= false;
	QLineEdit			*pLineEdit			= NULL;

	if (MCfgLaneToolConfig::getCfg()->doNotShowPlazaSpecificTab())
		return true;

	TRACE_W(QString("MMainDialog::savePlazaSpecParamsConfig() => START"));

	pSpecParams = MCfgLaneToolConfig::getCfg()->getSpecParams();

	iSelectedPlaza = m_uiPlazaNumForPlazaSpecParam;
	
	if (iSelectedPlaza < 1)
		return true;

	for (iIdx = 0; iIdx < pSpecParams->iNbPlazaItems; iIdx++)
	{
		if (pSpecParams->sPlazaParams[iIdx].iPlazaNum == iSelectedPlaza)
		{
			if (pSpecParams->sPlazaParams[iIdx].iNbItems > 0 && m_ui->tablePlazaSpecParams->rowCount() > 0)
			{
				for (iIdx2 = 0; iIdx2 < pSpecParams->sPlazaParams[iIdx].iNbItems; iIdx2++)
				{
					// registry path
					sprintf_s(szRegPath,
							sizeof(szRegPath),
							"%s%s",
							CSR_REG_KEYn_CSRBASE,
							pSpecParams->sPlazaParams[iIdx].sParams[iIdx2].sParamRegKey.toLatin1().data());

					if (pSpecParams->sPlazaParams[iIdx].sParams[iIdx2].bParamRegKeyMustExist && !doesParamRegKeyExist(szRegPath))
						continue;

					if (pSpecParams->sPlazaParams[iIdx].sParams[iIdx2].sParamType == QString(CFG_LANE_TOOL_PARAM_NUMBER))
					{
						DWORD dwParamValue = 0;

						if (m_ui->tablePlazaSpecParams->rowCount() <= iIdx2)
							dwParamValue = pSpecParams->sPlazaParams[iIdx].sParams[iIdx2].sDefaultValue.toULong();
						else
						{
							pLineEdit = (QLineEdit*)m_ui->tablePlazaSpecParams->cellWidget(iIdx2, 1);
							dwParamValue = pLineEdit->text().toULong();
						}

						// update specific parameter
						if (MRegUtils::updateDwordKey(szRegPath,
													pSpecParams->sPlazaParams[iIdx].sParams[iIdx2].sParamName.toLatin1().data(),
													dwParamValue) != MRegUtils::SUCCESS)
						{
							bNotUpdatetd = true;

							TRACE_W(QString("MMainDialog::savePlazaSpecParamsConfig() => MRegUtils::updateDwordKey(): Error update registry %1\\%2")
								.arg(szRegPath)
								.arg(pSpecParams->sPlazaParams[iIdx].sParams[iIdx2].sParamName.toLatin1().data()));
						}
					}
					else
					{
						QString sParamValue;

						if (m_ui->tablePlazaSpecParams->rowCount() <= iIdx2)
							sParamValue = pSpecParams->sPlazaParams[iIdx].sParams[iIdx2].sDefaultValue;
						else
						{
							pLineEdit = (QLineEdit*)m_ui->tablePlazaSpecParams->cellWidget(iIdx2, 1);
							sParamValue = pLineEdit->text();
						}

						// update specific parameter
						if (MRegUtils::updateStringKey(szRegPath,
													pSpecParams->sPlazaParams[iIdx].sParams[iIdx2].sParamName.toLatin1().data(),
													sParamValue.toLatin1().data()) != MRegUtils::SUCCESS)
						{
							bNotUpdatetd = true;

							TRACE_W(QString("MMainDialog::savePlazaSpecParamsConfig() => MRegUtils::updateStringKey(): Error update registry %1\\%2")
								.arg(szRegPath)
								.arg(pSpecParams->sPlazaParams[iIdx].sParams[iIdx2].sParamName.toLatin1().data()));
						}
					}
				}
			}
		}

		break;
	}

	return !bNotUpdatetd;
	TRACE_W(QString("MMainDialog::savePlazaSpecParamsConfig() => END  %1")
		.arg(bNotUpdatetd));
}

bool MMainDialog::getLaneSpecParamsConfig(int iPlazaNum, int iLaneNum)
{
	strucrt_spec_params	*pSpecParams	= NULL;
	bool				bNotUpdatetd	= false;
	bool				bFoundPlaza		= false;
	bool				bFoundLane		= false;
	int					iIdx			= 0;
	int					iIdx2			= 0;
	int					iParamIdx		= 0;
	int					iStatus			= 0;
	uint				uiParamValue	= 0;
	char				szRegPath[300]	= { 0 };
	QString				sParamValue;

	m_bReadActualCfg = true;

	if (!MCfgLaneToolConfig::getCfg()->doNotShowLaneSpecificTab())
	{
		pSpecParams = MCfgLaneToolConfig::getCfg()->getSpecParams();

		m_ui->tableLaneSpecParams->clearContents();
		m_ui->tableLaneSpecParams->setRowCount(0);

		m_uiPlazaNumForLaneSpecParam = 0;
		m_uiLaneNumForLaneSpecParam = 0;

		if (pSpecParams->sLaneParams.iNbItems > 0)
		{
			for (iIdx = 0; iIdx < pSpecParams->sLaneParams.iNbItems; iIdx++)
			{
				if (pSpecParams->sLaneParams.sPlaza[iIdx].iPlazaNum == iPlazaNum)
				{
					bFoundPlaza = true;

					for (iIdx2 = 0; iIdx2 < pSpecParams->sLaneParams.sPlaza[iIdx].iNbItems; iIdx2++)
					{
						if (pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].iLaneNum == iLaneNum)
						{
							bFoundLane = true;
							m_uiPlazaNumForLaneSpecParam = iPlazaNum;
							m_uiLaneNumForLaneSpecParam = iLaneNum;

							for (iParamIdx = 0; iParamIdx < pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].iNbItems; iParamIdx++)
							{
								// registry path
								sprintf_s(szRegPath,
									sizeof(szRegPath),
									"%s%s",
									CSR_REG_KEYn_CSRBASE,
									pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sParamRegKey.toLatin1().data());

								if (pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sParamType == QString(CFG_LANE_TOOL_PARAM_NUMBER))
								{
									// read specific parameter - number
									uiParamValue = MRegUtils::readDwordKey(szRegPath,
										pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sParamName.toLatin1().data(),
										&iStatus);

									if (iStatus != MRegUtils::SUCCESS)
									{
										sParamValue = pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sDefaultValue;

										bNotUpdatetd = true;

										TRACE_W(QString("MMainDialog::getPlazaSpecParamsConfig() => MRegUtils::readDwordKey(): Error read registry %1\\%2")
											.arg(szRegPath)
											.arg(pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sParamName.toLatin1().data()));
									}
									else
										sParamValue = QString("%1").arg(uiParamValue);
								}
								else
								{
									// read specific parameter - string
									sParamValue = MRegUtils::readStringKey(szRegPath,
										pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sParamName.toLatin1().data(),
										&iStatus);

									if (iStatus != MRegUtils::SUCCESS)
									{
										sParamValue = pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sDefaultValue;

										bNotUpdatetd = true;

										TRACE_W(QString("MMainDialog::getLaneTypeConfig() => MRegUtils::readStringKey(): Error read registry %1\\%2")
											.arg(szRegPath)
											.arg(pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sParamName.toLatin1().data()));
									}
								}

								addTableRow(m_ui->tableLaneSpecParams,
									QString(tr(pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sParamLabel.toLatin1().data())),
									sParamValue,
									pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sParamType);
							}

							break;
						}
					}

					break;
				}
			}
		}
	}

	if (!bFoundPlaza || !bFoundLane)
	{
		bNotUpdatetd = true;

		if (!bFoundPlaza)
		{
			TRACE_W(QString("MMainDialog::getLaneSpecParamsConfig() => Not found defined specific parameters for plaza number: %1")
				.arg(iPlazaNum));
		}
		else if(!bFoundLane)
		{
			TRACE_W(QString("MMainDialog::getLaneSpecParamsConfig() => Not found defined specific parameters for lane number: %1")
				.arg(iLaneNum));
		}
	}

	m_bReadActualCfg = false;

	return !bNotUpdatetd;
}

bool MMainDialog::saveLaneSpecParamsConfig()
{
	strucrt_spec_params	*pSpecParams		= NULL;
	int					iIdx				= 0;
	int					iIdx2				= 0;
	int					iIdx3				= 0;
	int					iSelectedPlaza		= 0;
	int					iSelectedLane		= 0;
	char				szRegPath[MAX_PATH] = { 0 };
	bool				bNotUpdatetd		= false;
	QLineEdit			*pLineEdit			= NULL;

	if (MCfgLaneToolConfig::getCfg()->doNotShowLaneSpecificTab())
		return true;

	TRACE_W(QString("MMainDialog::saveLaneSpecParamsConfig() => START"));

	pSpecParams = MCfgLaneToolConfig::getCfg()->getSpecParams();

	iSelectedPlaza = m_uiPlazaNumForLaneSpecParam;
	iSelectedLane = m_uiLaneNumForLaneSpecParam;
	
	if (iSelectedPlaza < 1 || iSelectedLane < 1)
		return true;

	for (iIdx = 0; iIdx < pSpecParams->sLaneParams.iNbItems; iIdx++)
	{
		if (pSpecParams->sLaneParams.sPlaza[iIdx].iPlazaNum == iSelectedPlaza)
		{
			for (iIdx2 = 0; iIdx2 < pSpecParams->sLaneParams.sPlaza[iIdx].iNbItems; iIdx2++)
			{
				if (pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].iLaneNum == iSelectedLane)
				{
					if (pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].iNbItems > 0 && m_ui->tableLaneSpecParams->rowCount() > 0)
					{
						for (iIdx3 = 0; iIdx3 < pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].iNbItems; iIdx3++)
						{
							// registry path
							sprintf_s(szRegPath,
									sizeof(szRegPath),
									"%s%s",
									CSR_REG_KEYn_CSRBASE,
									pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sParamRegKey.toLatin1().data());

							if (pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].bParamRegKeyMustExist && 
								!doesParamRegKeyExist(szRegPath))
								continue;

							if (pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sParamType == QString(CFG_LANE_TOOL_PARAM_NUMBER))
							{
								DWORD dwParamValue = 0;

								if (m_ui->tableLaneSpecParams->rowCount() <= iIdx3)
									dwParamValue = pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sDefaultValue.toULong();
								else
								{
									pLineEdit = (QLineEdit*)m_ui->tableLaneSpecParams->cellWidget(iIdx3, 1);
									dwParamValue = pLineEdit->text().toULong();
								}

								// update specific parameter
								if (MRegUtils::updateDwordKey(szRegPath,
										pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sParamName.toLatin1().data(),
										dwParamValue) != MRegUtils::SUCCESS)
								{
									bNotUpdatetd = true;

									TRACE_W(QString("MMainDialog::saveLaneSpecParamsConfig() => MRegUtils::updateDwordKey(): Error update registry %1\\%2")
										.arg(szRegPath)
										.arg(pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sParamName.toLatin1().data()));
								}
							}
							else
							{
								QString sParamValue;

								if (m_ui->tableLaneSpecParams->rowCount() <= iIdx3)
									sParamValue = pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sDefaultValue;
								else
								{
									pLineEdit = (QLineEdit*)m_ui->tableLaneSpecParams->cellWidget(iIdx3, 1);
									sParamValue = pLineEdit->text();
								}
									
								// update specific parameter
								if (MRegUtils::updateStringKey(szRegPath,
										pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sParamName.toLatin1().data(),
										sParamValue.toLatin1().data()) != MRegUtils::SUCCESS)
								{
									bNotUpdatetd = true;

									TRACE_W(QString("MMainDialog::saveLaneSpecParamsConfig() => MRegUtils::updateStringKey(): Error update registry %1\\%2")
										.arg(szRegPath)
										.arg(pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sParamName.toLatin1().data()));
								}
							}
						}
					}

					break;
				}
			}

			break;
		}
	}

	return !bNotUpdatetd;
	TRACE_W(QString("MMainDialog::saveLaneSpecParamsConfig() => END  %1")
		.arg(bNotUpdatetd));
}

void MMainDialog::mergeRegFiles(QString sRegDir, QString sRegFiles)
{
	int			iIdx = 0;
	QString		sCommand;
	QStringList sRegFileList;

	// get all registry files
	sRegFileList << sRegFiles.split("|");

	for (iIdx = 0; iIdx < sRegFileList.count(); iIdx++)
	{
		if (!sRegFileList.at(iIdx).isEmpty() && !MCfgLaneToolConfig::getCfg()->getRegEditCommand().isEmpty())
		{
			sCommand = MCfgLaneToolConfig::getCfg()->getRegEditCommand() + " " + sRegDir + "\\" + sRegFileList.at(iIdx);
			
			executeCommand(sCommand);
		}
	}
}

void MMainDialog::onPlazaNumChanged(int iValue)
{
	int		iPlazaIdx	= 0;
	int		iLaneIdx	= 0;
	bool	bUpdate		= false;

	bUpdate = true;
	if (m_bPlazaLaneFirstTimeChange && m_bInitDone)
	{
		m_bPlazaLaneFirstTimeChange = false;

		if (MCfgLaneToolConfig::getCfg()->askForColdStart() && 
			fileExists(MCfgLaneToolConfig::getCfg()->getContextFilePath()))
		{
			if (raiseMsgBox(this,
							QString(tr(CFG_LANE_TOOL_QUESTION_COLD_START)),
							QString(tr(CFG_LANE_TOOL_LABEL_YES)),
							QString(tr(CFG_LANE_TOOL_LABEL_NO)),
							QMessageBox::Question))
			{
				m_bMakeColdStart = true;
			}
			else
			{
				bUpdate = false;
				m_ui->spinBoxPlazaNumber->setValue(m_uiFirstPlazaNum);
			}
		}
		else 
		{ 
			if (!raiseMsgBox(this,
							QString(tr(CFG_LANE_TOOL_TABS_WILL_BE_UPDATED)),
							QString(tr(CFG_LANE_TOOL_LABEL_YES)),
							QString(tr(CFG_LANE_TOOL_LABEL_NO)),
							QMessageBox::Question))
			{
				bUpdate = false;
				m_ui->spinBoxPlazaNumber->setValue(m_uiFirstPlazaNum);
			}
		}
	}

	TRACE_W(QString("MMainDialog::onPlazaNumChanged()) -> iValue is %1 and Plaza Idx is %2")
		.arg(iValue)
		.arg(iPlazaIdx));

	if (bUpdate && 
		MCfgLaneToolConfig::getCfg()->getPlazaIndex(iValue, &iPlazaIdx) &&
		MCfgLaneToolConfig::getCfg()->getLaneIndex(iPlazaIdx, m_ui->spinBoxLaneNumber->value(), &iLaneIdx))
	{
		TRACE_W(QString("MMainDialog::onPlazaNumChanged()) -> iPlazaIdx is %1 and LaneNumber is %2 and iLaneIdx is %3")
			.arg(iPlazaIdx)
			.arg(m_ui->spinBoxLaneNumber->value())
			.arg(iLaneIdx));

		updateGeneralTab(iPlazaIdx, iLaneIdx);
		updateNetworkTab(iPlazaIdx, iLaneIdx);
		updateDevicesTab(iPlazaIdx, iLaneIdx);
		updatePlazaSpecParamTab(iValue);
		updateLaneSpecParamTab(iValue, m_ui->spinBoxLaneNumber->value());


		TRACE_W(QString("MMainDialog::onPlazaNumChanged()) => Plaza number => Plaza Number %1    Lane Number %2")
			.arg(iPlazaIdx)
			.arg(iLaneIdx));
	}
}

void MMainDialog::onLaneNumChanged(int iValue)
{
	int		iPlazaIdx	= 0;
	int		iLaneIdx	= 0;
	bool	bUpdate		= false;

	bUpdate = true;
	if (m_bPlazaLaneFirstTimeChange  && m_bInitDone)
	{
		m_bPlazaLaneFirstTimeChange = false;

		if (MCfgLaneToolConfig::getCfg()->askForColdStart() &&
			fileExists(MCfgLaneToolConfig::getCfg()->getContextFilePath()))
		{
			if (raiseMsgBox(this,
							QString(tr(CFG_LANE_TOOL_QUESTION_COLD_START)),
							QString(tr(CFG_LANE_TOOL_LABEL_YES)),
							QString(tr(CFG_LANE_TOOL_LABEL_NO)),
							QMessageBox::Question))
			{
				m_bMakeColdStart = true;
			}
			else
			{
				bUpdate = false;
				m_ui->spinBoxLaneNumber->setValue(m_uiFirstLaneNum);
			}
		}
		else
		{
			if (!raiseMsgBox(this,
							QString(tr(CFG_LANE_TOOL_TABS_WILL_BE_UPDATED)),
							QString(tr(CFG_LANE_TOOL_LABEL_YES)),
							QString(tr(CFG_LANE_TOOL_LABEL_NO)),
							QMessageBox::Question))
			{
				bUpdate = false;
				m_ui->spinBoxLaneNumber->setValue(m_uiFirstLaneNum);
			}
		}
	}

	if (bUpdate &&
		MCfgLaneToolConfig::getCfg()->getPlazaIndex(m_ui->spinBoxPlazaNumber->value(), &iPlazaIdx) &&
		MCfgLaneToolConfig::getCfg()->getLaneIndex(iPlazaIdx, iValue, &iLaneIdx))
	{
		updateGeneralTab(iPlazaIdx, iLaneIdx);
		updateNetworkTab(iPlazaIdx, iLaneIdx);
		updateDevicesTab(iPlazaIdx, iLaneIdx);
		updatePlazaSpecParamTab(m_ui->spinBoxPlazaNumber->value());
		updateLaneSpecParamTab(m_ui->spinBoxPlazaNumber->value(), iValue);
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
	updateMainToolboxSizeAndPos();
	updateAboutDlgSizeAndPos();

	//updateSettingsDlgSizeAndPos();
}

void MMainDialog::onBtnTools()
{
	if(	m_pMainToolBox->isVisible())	
		m_pMainToolBox->hide();
	else
	{
		updateMainToolboxSizeAndPos();
		m_pMainToolBox->show();
	}
}

void MMainDialog::updateMainToolboxSizeAndPos()
{
	QPoint ptContentPos = m_ui->frContent->pos(); //target position - (relative to parent "frBody")
	QPoint pt = m_ui->frBody->mapTo(this, ptContentPos); //top left of the target container

	QRect g = m_ui->frContent->geometry(); 
	m_pMainToolBox->setGeometry(g);
	m_pMainToolBox->move(pt); //align left in the target container
}

void MMainDialog::createMainToolBox()
{
	QList <MToolEntryConfigData*> *pLst = MCfgLaneToolConfig::getCfg()->getToolEntryCfgList();

	m_pMainToolBox = new MStyledToolBox(this);
	m_pMainToolBox->setObjectName(QString::fromUtf8("m_pMainToolBox"));
	m_pMainToolBox->setFixedWidth(m_ui->btnTools->width());
	m_pMainToolBox->hide();
	m_pMainToolBox->setAutoHide(true);
	m_pMainToolBox->setAutoHideTimeout(300);

	if (pLst->size() > 0)
	{
		QStringList sLstGroups;
		QPushButton *pCreatedButton = NULL;
		int iIndex;

		foreach(MToolEntryConfigData* p, *pLst)
		{
			iIndex = sLstGroups.indexOf(p->sGroupLabel);
			if(iIndex<0)
			{
				iIndex = m_pMainToolBox->addGroup(p->sGroupLabel);
				sLstGroups.append(p->sGroupLabel);
			}

			m_pMainToolBox->addGroupItem(iIndex,p->m_sAppLabel,&pCreatedButton);
			if(pCreatedButton!=NULL)
				connect(pCreatedButton, SIGNAL(clicked()), this, SLOT(onMenuItemClicked()));
			
			mapToolConfig.insert(pCreatedButton, p);

		}

		m_pMainToolBox->collapseToolboxMenu();
		m_pMainToolBox->retranslate();
	}
}

void MMainDialog::onMenuItemClicked()
{
	QPushButton *pSender = qobject_cast<QPushButton *>(sender());

	if(pSender!=NULL)
	{
		MToolEntryConfigData* p = mapToolConfig.value(pSender);
 		
		if(p->getCmdLine()!="")
		{
			if(p->eCmdType == MToolEntryConfigData::enuToolExec)
			{
				QStringList slstSartupParams;

				if(!m_LangPick->getSelectedLang().isEmpty())
					slstSartupParams << "-l" << m_LangPick->getSelectedLang();
				
				if(!p->getIniFilePAth().isEmpty())	
					slstSartupParams << "-c" << p->getIniFilePAth();

				QProcess::startDetached(p->getCmdLine(), slstSartupParams);
			}
			if(p->eCmdType == MToolEntryConfigData::enuShellExec)
			{
				QStringList slstSartupParams = p->sCmdParameters.split(' ')  ; 

				QProcess::startDetached(p->getCmdLine(), slstSartupParams);
			}
			else if(p->eCmdType == MToolEntryConfigData::enuOpenLocalUrl)
			{
				QString sLinkUrl =  p->getCmdLine();
				
				if(QDir::isRelativePath(sLinkUrl))
					sLinkUrl = QDir(sLinkUrl).absolutePath();

				QUrl sUrl = QUrl::fromLocalFile(sLinkUrl);

				QDesktopServices::openUrl(sUrl);	
			}
			else if(p->eCmdType == MToolEntryConfigData::enuOpenWebUrl)
			{
				QString sLinkUrl =  p->getCmdLine();
				QUrl sUrl = QUrl(sLinkUrl, QUrl::TolerantMode);

				QDesktopServices::openUrl(sUrl);	
			}
		}
	}
}

//---------------------------------------------------------------

void MMainDialog::createLangPick()
{
	m_LangPick = new MStyledLangPick(m_ui->frMenuLangPick);
	QList <MLangPickItemConfigData*> *pLst = MCfgLaneToolConfig::getCfg()->getLangPickConfig()->getLangPickCfgList();

//	MTranslator* pTr = MConfigMainIntf::getCfg()->getTranslator();
	
	foreach(MLangPickItemConfigData* p, *pLst)
	{
		//if(pTr->isAvailable(p->m_sLangId))
			m_LangPick->appendLanguage(p->m_sLangSelectingIMG, p->m_sLangSelectedIMG, p->m_sLangShortLabel, p->m_sLangId);
	}

	connect(m_LangPick, SIGNAL(selectionChanged(QString)), this, SLOT(onLangSelectionChanged(QString)));
	connect(m_ui->btnLanguages, SIGNAL(released()), m_LangPick,SLOT(toogleSelectingState()));
		
	QHBoxLayout * pLayout = (QHBoxLayout *)m_ui->frMenuLangPick->layout();
	if(pLayout!=NULL)
		pLayout->insertWidget(0,m_LangPick);

	m_LangPick->setSelectedLang(MConfig::getCfg()->getDefaultLanguageId());

}

void MMainDialog::onLangSelectionChanged(QString sLangID)
{
	MTranslator* pTr = MCfgLaneToolConfig::getCfg()->getTranslator();
	pTr->setLanguage(sLangID);
	qApp->installTranslator(pTr);

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
   m_pMainToolBox->retranslate();
   m_pAboutDlg->retranslate();

   //m_pSettingsDlg->retranslate();
}

void MMainDialog::initConfiguration()
{
	int						iTabCnt			= 0;
	int						iIdx			= 0;
	uint					uiPlazaNumber	= 0;
	uint					uiLaneNumber	= 0;
	int						iPlazaIdx		= 0;
	int						iLaneIdx		= 0;
	bool					bResult			= false;
	bool					bInitWithActual = false;
	QStringList				sHeaderLabels;
	strucrt_lane_type		*pLaneTypesCfg	= NULL;
	strucrt_lane_directions *pLaneDirection = NULL;

	m_bReadActualCfg = false;
	m_bMakeColdStart = false;
	m_bConfigSaved = false;
	m_bPlazaLaneFirstTimeChange = true;
	m_uiPlazaNumForPlazaSpecParam = 0;
	m_uiPlazaNumForLaneSpecParam = 0;
	m_uiLaneNumForLaneSpecParam = 0;
	m_uiFirstPlazaNum = 0;
	m_uiFirstLaneNum = 0;
	m_sLastSelectedAdapterName = QString();
	m_bInitDone = false;

	// general application parameters registry key
	sprintf_s(m_szGeneralAppParamReg,
			sizeof(m_szGeneralAppParamReg),
			"%s%s",
			CSR_REG_KEYn_CSRBASE,
			MCfgLaneToolConfig::getCfg()->getRegKeyAppParam().toLatin1().data());

	// TFT module registry key
	sprintf_s(m_szTftParamReg,
			sizeof(m_szTftParamReg),
			"%s%s",
			CSR_REG_KEYn_CSRBASE,
			MCfgLaneToolConfig::getCfg()->getRegKeyModuleTFT().toLatin1().data());

	// LAN module registry key
	sprintf_s(m_szLanParamReg,
			sizeof(m_szLanParamReg),
			"%s%s",
			CSR_REG_KEYn_CSRBASE,
			MCfgLaneToolConfig::getCfg()->getRegKeyModuleLAN().toLatin1().data());

	uiPlazaNumber = getCurrentPlazaNumber(m_szGeneralAppParamReg, &bResult);
	
	if (bResult)
		uiLaneNumber = getCurrentLaneNumber(m_szGeneralAppParamReg, &bResult);
	
	bInitWithActual = false;
	if (bResult && uiPlazaNumber > 0 && uiLaneNumber > 0)
	{
		if (MCfgLaneToolConfig::getCfg()->getPlazaIndex(uiPlazaNumber, &iPlazaIdx) &&
			MCfgLaneToolConfig::getCfg()->getLaneIndex(iPlazaIdx, uiLaneNumber, &iLaneIdx))
			bInitWithActual = true;

	}

	TRACE_W(QString("MMainDialog::initConfiguration => %1")
		.arg(bInitWithActual));
		
	pLaneTypesCfg = MCfgLaneToolConfig::getCfg()->getLaneTypesConfig();
	pLaneDirection = MCfgLaneToolConfig::getCfg()->getLaneDirections();

	iTabCnt = 0;
	if (MCfgLaneToolConfig::getCfg()->doNotShowGeneralTab())
		m_ui->tabWidget->removeTab(iTabCnt);
	else
	{
		if (MCfgLaneToolConfig::getCfg()->doNotShowChannelNumber())
		{
			m_ui->labelChannelNumber->setVisible(false);
			m_ui->spinBoxChannelNumber->setVisible(false);
		}
		else
		{
			m_ui->spinBoxChannelNumber->setMinimum(MCfgLaneToolConfig::getCfg()->getMinChannelNum());
			m_ui->spinBoxChannelNumber->setMaximum(MCfgLaneToolConfig::getCfg()->getMaxChannelNum());
		}
		
		if (MCfgLaneToolConfig::getCfg()->doNotShowLaneDirection())
		{
			m_ui->labelLaneDirection->setVisible(false);
			m_ui->comboBoxLaneDirection->setVisible(false);
		}

		if (MCfgLaneToolConfig::getCfg()->doNotShowFarePoint())
		{
			m_ui->labelFarePoint->setVisible(false);
			m_ui->lineEditFarePoint->setVisible(false);
		}
		else
			m_ui->lineEditFarePoint->setInputMask("999999999");

		if (MCfgLaneToolConfig::getCfg()->doNotShowDualLane())
			m_ui->checkBoxDualLane->setVisible(false);

		if (MCfgLaneToolConfig::getCfg()->doNotShowBtnGenActualCfg())
			m_ui->btnGeneralGetCurrent->setVisible(false);

		if (MCfgLaneToolConfig::getCfg()->doNotShowBtnGenDefaultCfg())
			m_ui->btnGeneralGetDefault->setVisible(false);

		m_ui->spinBoxPlazaNumber->setMinimum(MCfgLaneToolConfig::getCfg()->getMinPlazaNum());
		m_ui->spinBoxPlazaNumber->setMaximum(MCfgLaneToolConfig::getCfg()->getMaxPlazaNum());

		m_ui->spinBoxLaneNumber->setMinimum(MCfgLaneToolConfig::getCfg()->getMinLaneNum());
		m_ui->spinBoxLaneNumber->setMaximum(MCfgLaneToolConfig::getCfg()->getMaxLaneNum());

		m_ui->comboBoxLaneType->clear();
		for (iIdx = 0; iIdx < pLaneTypesCfg->iNbItems; iIdx++)
			m_ui->comboBoxLaneType->addItem(QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdx].sLaneType.toLatin1().data())));

		m_ui->comboBoxLaneDirection->clear();
		for (iIdx = 0; iIdx < pLaneDirection->iNbItems; iIdx++)
			m_ui->comboBoxLaneDirection->addItem(pLaneDirection->sLaneDirections[iIdx]);

		if (!bInitWithActual)
			updateGeneralTab(0, 0);
		
		m_uiFirstPlazaNum = m_ui->spinBoxPlazaNumber->value();
		m_uiFirstLaneNum = m_ui->spinBoxLaneNumber->value();

		iTabCnt++;
	}

	if (MCfgLaneToolConfig::getCfg()->doNotShowNetworkTab())
		m_ui->tabWidget->removeTab(iTabCnt);
	else
	{
		m_ui->lineEditIpAddress->setValidator(ipValidator(this));
		m_ui->lineEditSubnetMask->setValidator(ipValidator(this));
		m_ui->lineEditDefaultGateway->setValidator(ipValidator(this));

		getAllNetworkInterfaces();

		if (!bInitWithActual)
			updateNetworkTab(0, 0);

		iTabCnt++;
	}

	if (MCfgLaneToolConfig::getCfg()->doNotShowDevicesTab())
		m_ui->tabWidget->removeTab(iTabCnt);
	else
	{
		m_ui->treeListDevices->setSelectionMode(QAbstractItemView::NoSelection);

		if (MCfgLaneToolConfig::getCfg()->doNotShowBtnDevActualCfg())
			m_ui->btnDevGetCurrent->setVisible(false);

		if (MCfgLaneToolConfig::getCfg()->doNotShowBtnDevDefaultCfg())
			m_ui->btnDevGetDefault->setVisible(false);

		if (!bInitWithActual)
			updateDevicesTab(0, 0);

		iTabCnt++;
	}

	if (MCfgLaneToolConfig::getCfg()->doNotShowPlazaSpecificTab())
		m_ui->tabWidget->removeTab(iTabCnt);
	else
	{
		// header
		m_ui->tablePlazaSpecParams->setColumnCount(2);
		sHeaderLabels << QString(tr(CFG_LANE_TOOL_PARAMS)) << QString(tr(CFG_LANE_TOOL_PARAM_VALUE));
		m_ui->tablePlazaSpecParams->setHorizontalHeaderLabels(sHeaderLabels);

		m_ui->tablePlazaSpecParams->horizontalHeader()->setSectionsClickable(false);
		m_ui->tablePlazaSpecParams->horizontalHeader()->setStretchLastSection(true);
		m_ui->tablePlazaSpecParams->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		m_ui->tablePlazaSpecParams->verticalHeader()->setSectionsClickable(false);
		m_ui->tablePlazaSpecParams->verticalHeader()->setVisible(false);
//		m_ui->tablePlazaSpecParams->verticalHeader()->setResizeMode(QHeaderView::Stretch);
		m_ui->tablePlazaSpecParams->setCornerButtonEnabled(false);

		if (MCfgLaneToolConfig::getCfg()->doNotShowBtnPlazaActualCfg())
			m_ui->btnPlazaGetCurrent->setVisible(false);

		if (MCfgLaneToolConfig::getCfg()->doNotShowBtnPlazaDefaultCfg())
			m_ui->btnPlazaGetDefault->setVisible(false);

		if (!bInitWithActual)
		{
			if (!MCfgLaneToolConfig::getCfg()->doNotShowGeneralTab())
				updatePlazaSpecParamTab(m_ui->spinBoxPlazaNumber->value());
			else
			{
				strucrt_spec_params	*pSpecParams = MCfgLaneToolConfig::getCfg()->getSpecParams();

				if (pSpecParams->iNbPlazaItems > 0)
					updatePlazaSpecParamTab(pSpecParams->sPlazaParams[0].iPlazaNum);
			}
		}

		iTabCnt++;
	}

	if (MCfgLaneToolConfig::getCfg()->doNotShowLaneSpecificTab())
		m_ui->tabWidget->removeTab(iTabCnt);
	else
	{
		// header
		m_ui->tableLaneSpecParams->setColumnCount(2);
		sHeaderLabels << QString(tr(CFG_LANE_TOOL_PARAMS)) << QString(tr(CFG_LANE_TOOL_PARAM_VALUE));
		m_ui->tableLaneSpecParams->setHorizontalHeaderLabels(sHeaderLabels);

		m_ui->tableLaneSpecParams->horizontalHeader()->setSectionsClickable(false);
		m_ui->tableLaneSpecParams->horizontalHeader()->setStretchLastSection(true);
		m_ui->tableLaneSpecParams->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		m_ui->tableLaneSpecParams->verticalHeader()->setSectionsClickable(false);
		m_ui->tableLaneSpecParams->verticalHeader()->setVisible(false);
//		m_ui->tableLaneSpecParams->verticalHeader()->setResizeMode(QHeaderView::Stretch);
		m_ui->tableLaneSpecParams->setCornerButtonEnabled(false);

		if (MCfgLaneToolConfig::getCfg()->doNotShowBtnLaneActualCfg())
			m_ui->btnLaneGetCurrent->setVisible(false);

		if (MCfgLaneToolConfig::getCfg()->doNotShowBtnLaneDefaultCfg())
			m_ui->btnLaneGetDefault->setVisible(false);

		if (bInitWithActual)
		{
			if (!MCfgLaneToolConfig::getCfg()->doNotShowGeneralTab())
				updateLaneSpecParamTab(m_ui->spinBoxPlazaNumber->value(), m_ui->spinBoxLaneNumber->value());
			else
			{
				strucrt_spec_params	*pSpecParams = MCfgLaneToolConfig::getCfg()->getSpecParams();

				if (pSpecParams->sLaneParams.iNbItems > 0 && pSpecParams->sLaneParams.sPlaza[0].iNbItems > 0)
					updateLaneSpecParamTab(pSpecParams->sLaneParams.sPlaza[0].iPlazaNum, pSpecParams->sLaneParams.sPlaza[0].sLane[0].iLaneNum);
			}
		}

		iTabCnt++;
	}

	if (bInitWithActual)
	{
		getGeneralConfig();
		getNetworkConfig();
		getLaneTypeConfig();
		getSpecParamsConfig(m_ui->spinBoxPlazaNumber->value(), 
							m_ui->spinBoxLaneNumber->value());
	}

	m_ui->tabWidget->setCurrentIndex(0);

	m_bInitDone = true;
}

void MMainDialog::updateGeneralTab(int iPlazaIdx, int iLaneIdx)
{
	int						iIdx			= 0;
	strucrt_lane_type		*pLaneTypesCfg	= NULL;
	strucrt_lane_directions *pLaneDirection	= NULL;
	strucrt_manual_cfg		*pManualConfig	= NULL;
	QString					szDefaultScriptFile;
	QString					szScriptFile;

	if (m_bReadActualCfg)
		return;

	pLaneTypesCfg = MCfgLaneToolConfig::getCfg()->getLaneTypesConfig();
	pLaneDirection = MCfgLaneToolConfig::getCfg()->getLaneDirections();
	pManualConfig = MCfgLaneToolConfig::getCfg()->getManualConfig();

	// default configuration
	if (pManualConfig->iNbItems > 0 && pManualConfig->sPlazaConfig[iPlazaIdx].iNbItems > 0)
	{
		if (pManualConfig->sPlazaConfig[iPlazaIdx].iPlazaNum < MCfgLaneToolConfig::getCfg()->getMinPlazaNum() ||
			pManualConfig->sPlazaConfig[iPlazaIdx].iPlazaNum > MCfgLaneToolConfig::getCfg()->getMaxPlazaNum() ||
			pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].iLaneNum < MCfgLaneToolConfig::getCfg()->getMinLaneNum() ||
			pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].iLaneNum > MCfgLaneToolConfig::getCfg()->getMaxLaneNum())
		{
			TRACE_W(QString("MMainDialog::initConfiguration()) => Error Lane or Plaza number => Lane Number: %1, min: %2, max: %3, Plaza Number: %4, min: %5, max: %6!")
				.arg(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].iLaneNum)
				.arg(MCfgLaneToolConfig::getCfg()->getMinLaneNum())
				.arg(MCfgLaneToolConfig::getCfg()->getMaxLaneNum())
				.arg(pManualConfig->sPlazaConfig[iPlazaIdx].iPlazaNum)
				.arg(MCfgLaneToolConfig::getCfg()->getMinPlazaNum())
				.arg(MCfgLaneToolConfig::getCfg()->getMaxPlazaNum()));
		}
		else
		{
			if (iPlazaIdx < pManualConfig->iNbItems && iLaneIdx < pManualConfig->sPlazaConfig[iPlazaIdx].iNbItems)
			{
				// set default plaza number
				m_ui->spinBoxPlazaNumber->setValue(pManualConfig->sPlazaConfig[iPlazaIdx].iPlazaNum);

				// set default lane number
				m_ui->spinBoxLaneNumber->setValue(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].iLaneNum);

				// set default channel number
				if (!MCfgLaneToolConfig::getCfg()->doNotShowChannelNumber())
					m_ui->spinBoxChannelNumber->setValue(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].iLaneNum);

				// set default plaza name
				m_ui->lineEditPlazaName->setText(pManualConfig->sPlazaConfig[iPlazaIdx].sPlazaName);

				// set default lane name
				m_ui->lineEditLaneName->setText(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sLaneName);

				// set default lane type
				for (iIdx = 0; iIdx < pLaneTypesCfg->iNbItems; iIdx++)
				{
					if (QString(tr(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sLaneType.toLatin1().data())) 
						== QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdx].sLaneType.toLatin1().data())))
					{
						m_ui->comboBoxLaneType->setCurrentIndex(iIdx);
						break;
					}
				}

				// set default lane direction
				if (!MCfgLaneToolConfig::getCfg()->doNotShowLaneDirection())
				{
					for (iIdx = 0; iIdx < pLaneDirection->iNbItems; iIdx++)
					{
						if (pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sLaneDirection == pLaneDirection->sLaneDirections[iIdx])
						{
							m_ui->comboBoxLaneDirection->setCurrentIndex(iIdx);
							break;
						}
					}
				}

				// set default fare point
				if (!MCfgLaneToolConfig::getCfg()->doNotShowFarePoint())
				{
					m_ui->lineEditFarePoint->setText(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sFarePoint);
				}

				// is  dual lane?
				if (!MCfgLaneToolConfig::getCfg()->doNotShowDualLane())
					m_ui->checkBoxDualLane->setChecked(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].bDualLane);

				//set default script file
				if (pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sScriptCommand.size()<3)
				{
					szDefaultScriptFile = MCfgLaneToolConfig::getCfg()->getDefaultScriptFile();
					szScriptFile = szDefaultScriptFile;
				}
				else
				{
					szScriptFile = pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sScriptCommand;
				}
			}
		}
	}
}

void MMainDialog::clearNetworkTab()
{
	m_ui->lineEditComputerName->clear();
	m_ui->lineEditWorkGroup->clear();
	m_ui->lineEditFileServer->clear();
	m_ui->lineEditMessageServer->clear();
	m_ui->comboBoxNetworkCard->clear();
	m_ui->checkBoxAutoIp->setChecked(false);
	m_ui->lineEditIpAddress->clear();
	m_ui->lineEditSubnetMask->clear();
	m_ui->lineEditDefaultGateway->clear();
}

void MMainDialog::updateNetworkTab(int iPlazaIdx, int iLaneIdx)
{
	int					iIdx			= 0;
	strucrt_manual_cfg	*pManualConfig	= NULL;

	m_bNetTabActualCfg = false;

	clearNetworkTab();

	resetNetCfgToApply();

	if (m_bReadActualCfg || MCfgLaneToolConfig::getCfg()->doNotShowNetworkTab())
		return;

	pManualConfig = MCfgLaneToolConfig::getCfg()->getManualConfig();

	// default configuration
	if (pManualConfig->iNbItems > 0 && pManualConfig->sPlazaConfig[iPlazaIdx].iNbItems > 0)
	{
		m_ui->lineEditFileServer->setText(pManualConfig->sPlazaConfig[iPlazaIdx].sPlazaFileServer);
		m_ui->lineEditMessageServer->setText(pManualConfig->sPlazaConfig[iPlazaIdx].sPlazaMsgServer);

		if (pManualConfig->sPlazaConfig[iPlazaIdx].iNbItems > 0)
		{
			m_ui->lineEditComputerName->setText(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sComputerName);
			m_ui->lineEditWorkGroup->setText(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sWorkGroup);

			if (pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].iNbNetCfg > 0)
			{
				for (iIdx = 0; iIdx < pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].iNbNetCfg; iIdx++)
					m_ui->comboBoxNetworkCard->addItem(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sNetworkConfig[iIdx].sAdapterName);

				updateNetCfgToApplyWithDefaultCfg(iPlazaIdx, iLaneIdx);

				m_ui->comboBoxNetworkCard->setCurrentIndex(0);
				updateAndShowIpAddressData(&sNetCfgToApply[0]);
			}
		}
	}
}

void MMainDialog::updateDevicesTab(int iPlazaIdx, int iLaneIdx)
{
	int						iIdx			= 0;
	int						iIdx2			= 0;
	int						iIdx3			= 0;
	strucrt_lane_type		*pLaneTypesCfg	= NULL;
	strucrt_manual_cfg		*pManualConfig	= NULL;
	QTreeWidget				*pTree			= m_ui->treeListDevices;

	if (m_bReadActualCfg)
		return;

	pLaneTypesCfg = MCfgLaneToolConfig::getCfg()->getLaneTypesConfig();
	pManualConfig = MCfgLaneToolConfig::getCfg()->getManualConfig();

	pTree->clear();
	m_sLaneTypeForDevParam = QString();

	// default configuration
	if (pManualConfig->iNbItems > 0 && pManualConfig->sPlazaConfig[iPlazaIdx].iNbItems > 0)
	{
		if (iPlazaIdx < pManualConfig->iNbItems && iLaneIdx < pManualConfig->sPlazaConfig[iPlazaIdx].iNbItems)
		{
			for (iIdx = 0; iIdx< pLaneTypesCfg->iNbItems; iIdx++)
			{
				if (QString(tr(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sLaneType.toLatin1().data())) ==
					QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdx].sLaneType.toLatin1().data())))
				{
					m_sLaneTypeForDevParam = QString(tr(pManualConfig->sPlazaConfig[iPlazaIdx].sLaneConfig[iLaneIdx].sLaneType.toLatin1().data()));

					if (!MCfgLaneToolConfig::getCfg()->doNotShowDevicesTab() && pLaneTypesCfg->sLaneTypeItem[iIdx].iNbItems > 0)
					{
						for (iIdx2 = 0; iIdx2 < pLaneTypesCfg->sLaneTypeItem[iIdx].iNbItems; iIdx2++)
						{
							QTreeWidgetItem *pTopLevel = new QTreeWidgetItem();

							pTopLevel->setText(0, QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sLabel.toLatin1().data())));

							for (iIdx3 = 0; iIdx3 < pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].iNbItems; iIdx3++)
							{
								QTreeWidgetItem *pItem = new QTreeWidgetItem();

								pItem->setText(0, QString(tr(pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].sLabel.toLatin1().data())));
								pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable);
								pItem->setCheckState(0, (pLaneTypesCfg->sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].bIsDefaultChecked) ? Qt::Checked : Qt::Unchecked);

								pTopLevel->addChild(pItem);
							}

							pTree->addTopLevelItem(pTopLevel);
						}

						pTree->expandAll();
					}
				}
			}
		}
	}
}

void MMainDialog::updatePlazaSpecParamTab(int iPlazaNum)
{
	strucrt_spec_params	*pSpecParams	= NULL;
	int					iIdx			= 0;
	int					iParamIdx		= 0;

	if (m_bReadActualCfg || MCfgLaneToolConfig::getCfg()->doNotShowPlazaSpecificTab())
		return;

	pSpecParams = MCfgLaneToolConfig::getCfg()->getSpecParams();

	m_ui->tablePlazaSpecParams->setRowCount(0);
	m_uiPlazaNumForPlazaSpecParam = 0;

	if (pSpecParams->iNbPlazaItems > 0)
	{
		for (iIdx = 0; iIdx < pSpecParams->iNbPlazaItems; iIdx++)
		{
			if (pSpecParams->sPlazaParams[iIdx].iPlazaNum == iPlazaNum)
			{	
				m_uiPlazaNumForPlazaSpecParam = iPlazaNum;

				for (iParamIdx = 0; iParamIdx < pSpecParams->sPlazaParams[iIdx].iNbItems; iParamIdx++)
				{
					addTableRow(m_ui->tablePlazaSpecParams,
						QString(tr(pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sParamLabel.toLatin1().data())),
						QString(tr(pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sDefaultValue.toLatin1().data())),
						pSpecParams->sPlazaParams[iIdx].sParams[iParamIdx].sParamType);
				}

				break;
			}
		}
	}
}

void MMainDialog::updateLaneSpecParamTab(int iPlazaNum, int iLaneNum)
{
	strucrt_spec_params	*pSpecParams = NULL;
	int					iIdx = 0;
	int					iIdx2 = 0;
	int					iParamIdx = 0;

	if (m_bReadActualCfg || MCfgLaneToolConfig::getCfg()->doNotShowLaneSpecificTab())
		return;

	pSpecParams = MCfgLaneToolConfig::getCfg()->getSpecParams();

	m_ui->tableLaneSpecParams->clearContents();
	m_ui->tableLaneSpecParams->setRowCount(0);
	m_uiPlazaNumForLaneSpecParam = 0;
	m_uiLaneNumForLaneSpecParam = 0;

	if (pSpecParams->sLaneParams.iNbItems > 0)
	{
		for (iIdx = 0; iIdx < pSpecParams->sLaneParams.iNbItems; iIdx++)
		{
			if (pSpecParams->sLaneParams.sPlaza[iIdx].iPlazaNum == iPlazaNum)
			{
				for (iIdx2 = 0; iIdx2 < pSpecParams->sLaneParams.sPlaza[iIdx].iNbItems; iIdx2++)
				{
					if (pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].iLaneNum == iLaneNum)
					{
						m_uiPlazaNumForLaneSpecParam = iPlazaNum;
						m_uiLaneNumForLaneSpecParam = iLaneNum;

						for (iParamIdx = 0; iParamIdx < pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].iNbItems; iParamIdx++)
						{
							addTableRow(m_ui->tableLaneSpecParams,
								QString(tr(pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sParamLabel.toLatin1().data())),
								QString(tr(pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sDefaultValue.toLatin1().data())),
								pSpecParams->sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iParamIdx].sParamType);
						}
					}
				}
			}
		}
	}
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

	MHelpFuncs::setFileCSSToWidget(MCfgLaneToolConfig::getCfg()->getDialogStylesheetPath(), &msgBox);

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

QString MMainDialog::getCurrentPlazaName(CHAR *pRegPath, bool *pStatus)
{
	int		iStatus = 0;
	QString	sValue;

	*pStatus = true;

	// read plaza name
	sValue = MRegUtils::readStringKey(pRegPath,
									MCfgLaneToolConfig::getCfg()->getRegValPlazaName().toLatin1().data(),
									&iStatus);

	if (sValue.isEmpty() || iStatus != MRegUtils::SUCCESS)
	{
		*pStatus = false;

		TRACE_W(QString("MMainDialog::getCurrentPlazaName() => MRegUtils::readStringKey(): Error read registry %1\\%2")
			.arg(pRegPath)
			.arg(MCfgLaneToolConfig::getCfg()->getRegValPlazaName().toLatin1().data()));
	}

	return sValue;
}

uint MMainDialog::getCurrentPlazaNumber(CHAR *pRegPath, bool *pStatus)
{
	int		iStatus = 0;
	uint	uiValue = 0;

	// read plaza number
	uiValue = MRegUtils::readDwordKey(pRegPath,
									MCfgLaneToolConfig::getCfg()->getRegValPlazaNumber().toLatin1().data(),
									&iStatus);

	*pStatus = true;

	if (iStatus != MRegUtils::SUCCESS)
	{
		*pStatus = false;

		TRACE_W(QString("MMainDialog::getGeneralConfig() => MRegUtils::readDwordKey(): Error read registry %1\\%2")
			.arg(pRegPath)
			.arg(MCfgLaneToolConfig::getCfg()->getRegValPlazaNumber().toLatin1().data()));
	}

	return uiValue;
}

uint MMainDialog::getCurrentLaneNumber(CHAR *pRegPath, bool *pStatus)
{
	int		iStatus = 0;
	uint	uiValue = 0;

	// read lane number
	uiValue = MRegUtils::readDwordKey(pRegPath,
									MCfgLaneToolConfig::getCfg()->getRegValLaneNumber().toLatin1().data(),
									&iStatus);

	*pStatus = true;

	if (iStatus != MRegUtils::SUCCESS)
	{
		*pStatus = false;

		TRACE_W(QString("MMainDialog::getGeneralConfig() => MRegUtils::readDwordKey(): Error read registry %1\\%2")
			.arg(pRegPath)
			.arg(MCfgLaneToolConfig::getCfg()->getRegValLaneNumber().toLatin1().data()));
	}

	return uiValue;
}

bool MMainDialog::fileExists(QString sPath)
{
	QFileInfo checkFile(sPath);
	
	// check if file exists and if yes: Is it really a file and no directory?
	if (checkFile.exists() && checkFile.isFile()) 
		return true;
	
	return false;
}

QRegularExpressionValidator* MMainDialog::ipValidator(QWidget *pParent)
{
	QString sIpRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";

	QRegularExpression ipRegex("^" + sIpRange
					+ "\\." + sIpRange
					+ "\\." + sIpRange
					+ "\\." + sIpRange + "$");

	QRegularExpressionValidator*ipValidator = new QRegularExpressionValidator(ipRegex, pParent);

	return ipValidator;
}

QLineEdit * MMainDialog::createLineEditForSpecParam(QWidget *pParent, QString sParamType)
{
	QLineEdit *pLineEdit = new QLineEdit(pParent);

	if (sParamType == QString(CFG_LANE_TOOL_PARAM_NUMBER))
		pLineEdit->setInputMask(MCfgLaneToolConfig::getCfg()->getSpecParamNumberInputMask());
	else if (sParamType == QString(CFG_LANE_TOOL_PARAM_STRING))
		pLineEdit->setMaxLength(MCfgLaneToolConfig::getCfg()->getSpecParamStringMaxLength());
	else if (sParamType == QString(CFG_LANE_TOOL_PARAM_IP_ADDRESS))
		pLineEdit->setValidator(ipValidator(pParent));

	return pLineEdit;
}

void MMainDialog::addTableRow(QTableWidget *pTable, QString sParamLabel, QString sParamValue, QString sParamType)
{
	int iRow = 0;

	QTableWidgetItem *pItemParamLabel = new QTableWidgetItem(QString(tr(sParamLabel.toLatin1().data())));

	QLineEdit *pLineEdit = createLineEditForSpecParam(NULL, sParamType);

	pLineEdit->setText(QString(tr(sParamValue.toLatin1().data())));

	pTable->insertRow(pTable->rowCount());
	iRow = pTable->rowCount() - 1;

	pItemParamLabel->setFlags(pItemParamLabel->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
	pTable->setItem(iRow, 0, pItemParamLabel);
	pTable->setCellWidget(iRow, 1, pLineEdit);
}

bool MMainDialog::getAllNetworkInterfaces()
{
	PIP_ADAPTER_INFO		pAdapterInfo = NULL;
	PIP_ADAPTER_INFO		pAdapter = NULL;
	PIP_ADDR_STRING			pAddressInfo = NULL;
	DWORD					dwRetVal = 0;
	ULONG					ulOutBufLen, ulCnt = 0;
	DWORD					dwCounter = 0;
	ULONG					flags, outBufLen = 0, family;
	PIP_ADAPTER_ADDRESSES	pAddresses;
	PIP_ADAPTER_ADDRESSES	pCurrAddresses;

	ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulOutBufLen);
	if (pAdapterInfo == NULL)
	{
		TRACE_W(QString("MMainDialog::getAllNetworkInterfaces() => Memory allocation faild for adapter info!"));
		return false;
	}
		
	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);
		pAdapterInfo = NULL;

		pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulOutBufLen);
		if (pAdapterInfo == NULL)
		{
			TRACE_W(QString("MMainDialog::getAllNetworkInterfaces() => Memory allocation faild for adapter info!"));
			return false;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			strcpy_s(sAdaptersInfo[ulCnt].AdapterName, sizeof(sAdaptersInfo[ulCnt].AdapterName), pAdapter->AdapterName);
			strcpy_s(sAdaptersInfo[ulCnt].Description, sizeof(sAdaptersInfo[ulCnt].Description), pAdapter->Description);
			sAdaptersInfo[ulCnt].AddressLength = pAdapter->AddressLength;
			memcpy(sAdaptersInfo[ulCnt].Address, pAdapter->Address, sizeof(sAdaptersInfo[ulCnt].Address));
			sAdaptersInfo[ulCnt].Index = pAdapter->Index;
			sAdaptersInfo[ulCnt].Type = pAdapter->Type;
			sAdaptersInfo[ulCnt].DhcpEnabled = pAdapter->DhcpEnabled;

			sAdaptersInfo[ulCnt].bUpdateIpSettings = false;

			sAdaptersInfo[ulCnt].dwNbIpAddresses = 0;
			pAddressInfo = &pAdapter->IpAddressList;
			while (pAddressInfo)
			{
				if (sAdaptersInfo[ulCnt].dwNbIpAddresses >= MAX_NB_IP_ADDRESSES)
					break;

				strcpy_s((char*)sAdaptersInfo[ulCnt].sIpInfo[sAdaptersInfo[ulCnt].dwNbIpAddresses].IpAddress, 
					sizeof(sAdaptersInfo[ulCnt].sIpInfo[sAdaptersInfo[ulCnt].dwNbIpAddresses].IpAddress),
					pAddressInfo->IpAddress.String);

				strcpy_s((char*)sAdaptersInfo[ulCnt].sIpInfo[sAdaptersInfo[ulCnt].dwNbIpAddresses].IpMask, 
					sizeof(sAdaptersInfo[ulCnt].sIpInfo[sAdaptersInfo[ulCnt].dwNbIpAddresses].IpMask),
					pAddressInfo->IpMask.String);

				sAdaptersInfo[ulCnt].dwNbIpAddresses++;
				pAddressInfo = pAddressInfo->Next;
			}
			
			strcpy_s((char*)sAdaptersInfo[ulCnt].Gateway, sizeof(sAdaptersInfo[ulCnt].Gateway), pAdapter->GatewayList.IpAddress.String);
			
			if (sAdaptersInfo[ulCnt].DhcpEnabled)
				strcpy_s((char*)sAdaptersInfo[ulCnt].DhcpServer, sizeof(sAdaptersInfo[ulCnt].DhcpServer), pAdapter->DhcpServer.IpAddress.String);

			pAdapter = pAdapter->Next;

			ulCnt++;

			if (ulCnt >= MAX_NB_IP_ADAPTERS)
				break;
		}

		ulNbAdapters = ulCnt;
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);
		pAdapterInfo = NULL;

		//Get adapter friendly name
		//////////////////////////////////////////////////////////////////////////
		// Set the flags to pass to GetAdaptersAddresses
		flags = GAA_FLAG_INCLUDE_PREFIX;
		// default to unspecified address family (both)
		family = AF_UNSPEC;
		pAddresses = NULL;

		pCurrAddresses = NULL;

		outBufLen = sizeof(IP_ADAPTER_ADDRESSES);
		pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
		if (pAddresses == NULL)
		{
			TRACE_W(QString("MMainDialog::getAllNetworkInterfaces() => Memory reallocation faild for address!"));
			return false;
		}

		// Make an initial call to GetAdaptersAddresses to get the
		// size needed into the outBufLen variable
		if (GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW)
		{
			free(pAddresses);
			pAddresses = NULL;

			pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
		}

		if (pAddresses == NULL)
		{
			TRACE_W(QString("MMainDialog::getAllNetworkInterfaces() => Memory reallocation faild for address!"));
			return false;
		}

		dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);
		if (dwRetVal == NO_ERROR)
		{
			// If successful copy data
			pCurrAddresses = pAddresses;
			while (pCurrAddresses)
			{
				for (dwCounter = 0; dwCounter<ulNbAdapters; dwCounter++)
				{
					if (strcmp(sAdaptersInfo[dwCounter].AdapterName, pCurrAddresses->AdapterName) == 0)
					{
						//strcpy(sAdaptersInfo[ulCnt].AdapterName, pCurrAddresses->AdapterName);
						sprintf_s(sAdaptersInfo[dwCounter].FriendlyName, sizeof(sAdaptersInfo[dwCounter].FriendlyName), "%wS", pCurrAddresses->FriendlyName);
						break;
					}
				}

				pCurrAddresses = pCurrAddresses->Next;
			}
		}
		else
		{
			TRACE_W(QString("MMainDialog::getAllNetworkInterfaces() => Function GetAdaptersAddresses() failed with error: %1").arg(dwRetVal));
		}

		free(pAddresses);
		pAddresses = NULL;

		return true;
	}

	HeapFree(GetProcessHeap(), 0, pAdapterInfo);
	pAdapterInfo = NULL;

	return false;
}

void MMainDialog::executeCommand(QString sCommand)
{
	ExecuteCmd(sCommand);
}

bool MMainDialog::doesParamRegKeyExist(char *pRegPath)
{
	int		iKeyIdx			= 0;
	DWORD	dwKeySize		= 0;
	CHAR	szRegPath[300]	= { 0 };
	CHAR	szKeyName[100]	= { 0 };
	CHAR	*pLast			= NULL;
	bool	bFound			= false;

	if (pRegPath == NULL)
		return false;

	pLast = strrchr(pRegPath, '\\');
	if (pLast == NULL)
		return false;

	memcpy(szRegPath, pRegPath, pLast - pRegPath);

	pLast++;
	if (pLast == NULL)
		return false;

	iKeyIdx = 0;  dwKeySize = sizeof(szKeyName);
	while (REG_Enum_Cles(HKEY_LOCAL_MACHINE, szRegPath, iKeyIdx, szKeyName, &dwKeySize) == ERROR_SUCCESS)
	{
		if (QString(pLast) == QString(szKeyName))
		{
			bFound = true;
			break;
		}

		iKeyIdx++; dwKeySize = sizeof(szKeyName);
	}

	return bFound;
}


void MMainDialog::ExecuteCmd(QString sCommand)
{
	QProcess cmdExecute;
	bool bIsFinishedOk = false;

	TRACE_W(QString("MMainDialog::ExecuteCommandRstrSettings() => Command %1").arg(sCommand));

	cmdExecute.start(sCommand);
	if (cmdExecute.state() == QProcess::Starting)
		cmdExecute.waitForStarted();

	if (cmdExecute.state() == QProcess::Running)
	{
		bIsFinishedOk = cmdExecute.waitForFinished();
	}
	else
		TRACE_W(QString("MMainDialog::ExecuteCommandRstrSettings() => ERROR: Unable to start!"));

	if (bIsFinishedOk)
	{
		QByteArray ba = cmdExecute.readAllStandardError();
		if (ba.size() > 0)
		{
			TRACE_W(QString("MMainDialog::ExecuteCommandRstrSettings()ERROR: %1").arg(ba.data()));
		}
		else
		{
			TRACE_W(QString("MMainDialog::ExecuteCommandRstrSettings() => OK"));
		}
	}
	else
	{
		TRACE_W(QString("MMainDialog::ExecuteCommandRstrSettings() => Process timeout"));
	}

}

void MMainDialog::ExecuteCommandRstrSettings(strucrt_lane_cfg *pFileToProcess)
{
	QString szCommand;
	QString szSourcePath;

	szCommand = pFileToProcess->sScriptCommand;
	if (szCommand.isEmpty())
		return;

	szSourcePath = MCfgLaneToolConfig::getCfg()->getScriptPath();

	szCommand = szSourcePath.append(QString("\\")).append(szCommand);

	if (szCommand.trimmed().size() >0)
	{
		ExecuteCmd(szCommand);
	}
}

/*-------------------------------- END OF FILE ------------------------------*/