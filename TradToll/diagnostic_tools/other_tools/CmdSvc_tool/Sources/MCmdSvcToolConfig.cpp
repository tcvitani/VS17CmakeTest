/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 CMDSVC_tool												 */
/* FILE:	 MCmdSvcToolConfig.cpp											 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QApplication>
#include <QSettings>
#include <QString>
#include <QFile>
#include <QMessageBox>

#include "MDefines.h"
#include "MCmdSvcToolConfig.h"
#include "MTracer.h"
#include "MLangPickConfig.h"

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/
#define CMDSVC_CONFIG_FILE_PATH					"csr_CmdSvc_Tool.ini"
#define CMDSVC_INI_INSTANCE_NAME				"InstanceName"



/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE:      -----------------------------*/

MCmdSvcToolConfig::MCmdSvcToolConfig(QString sAppBaseConfigKeyName) :
MConfig(sAppBaseConfigKeyName), m_pLangPickCfg(NULL)
{

}

MCmdSvcToolConfig::~MCmdSvcToolConfig()
{
	if (m_pLangPickCfg != NULL)
		delete m_pLangPickCfg;
}

QString MCmdSvcToolConfig::getDefaultConfigFilePath()
{
	return QString("%1/%2")
		.arg(QApplication::applicationDirPath())
		.arg(CMDSVC_CONFIG_FILE_PATH);
}

bool MCmdSvcToolConfig::loadConfig()
{
	QString			sValue;
	QStringList		sChildKeys;

	TRACE_D("MCmdSvcToolConfig::loadConfig()");

	if (!MConfig::loadConfig())
		return false;

	// GET .INI CONFIG FILE
	QSettings sSettings(getConfigFilePath(), QSettings::IniFormat);
	sSettings.sync();

	//////////////////////////////////////////////////////////////////////////////////

	sSettings.beginGroup(TOOL_GEN_SETTINGS_INI_FILE_KEY);
	m_sInstanceName = sSettings.value(CMDSVC_INI_INSTANCE_NAME, "").toString();

	sSettings.endGroup();

	//get language pick settings from the same ini file 
	m_pLangPickCfg = new MLangPickConfig(getConfigFilePath());

	if (!m_pLangPickCfg->loadConfig())
	{
		TRACE_W(QString("MCmdSvcToolConfig()::loadConfig() => Error loading lang pick config from file: %1")
			.arg(getConfigFilePath()));

		QMessageBox::critical(NULL,
			MHelpFuncs::getProcessFileName(),
			QString("MCmdSvcToolConfig()::loadConfig() Error loading lang pick config from file: %1")
			.arg(getConfigFilePath()));

		return false;
	}



	return TRUE;
}


/*-------------------------------- END OF FILE ------------------------------*/