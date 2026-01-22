/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:     MLangPickConfig.cpp											 */
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

#include "MLangPickConfig.h"
#include "MHelpFuncs.h"
#include "MTracer.h"

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/
#define INI_LANGUAGE_IDS					"APP_LANGUAGE_IDS"
#define INI_LANGUAGE_STATE_SELECTING_IMG	"STATE_SELECTING_IMG"
#define INI_LANGUAGE_STATE_SELECTED_IMG		"STATE_SELECTED_IMG"
#define INI_LANGUAGE_LANG_SHORT_LABEL		"LANG_SHORT_LABEL"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

MLangPickConfig::~MLangPickConfig()
{
	while (!m_lstLangPickCfg.isEmpty())
		delete m_lstLangPickCfg.takeFirst();
 
}


bool MLangPickConfig::loadConfig()
{
	QString			sValue;
	QStringList		sChildKeys;	
	QString			sLangFilePath;
	QString			sLanguage;

	TRACE_D("MLangPickConfig::loadConfig()" );

	// CHECK THAT .INI FILE EXIST
	QFile checkConfig(m_sConfigFilePath);
	if (!checkConfig.exists())
	{			
		TRACE_W(QString("MLangPickConfig::loadConfig Error loading config file: %1")
			.arg(m_sConfigFilePath));
		
		QMessageBox::critical(NULL, 
			"MLangPickConfig",
			QString("MLangPickConfig::loadConfig:Error loading config file %1")
				.arg(m_sConfigFilePath));
		
		return false;
	}
	
	// GET .INI FILE
	QSettings sSettings (m_sConfigFilePath, QSettings::IniFormat);
	sSettings.sync();

	//-----------------------------------------------------------------	
	sSettings.beginGroup(INI_LANGUAGE_IDS);
	
	QStringList slstLangIds;

    sChildKeys = sSettings.childKeys();
    foreach (const QString &childKey, sChildKeys)
        slstLangIds << sSettings.value(childKey).toString();

    sSettings.endGroup();

	foreach(QString sLangID, slstLangIds)
	{
		sSettings.beginGroup(sLangID);
			
			MLangPickItemConfigData * pNew = new MLangPickItemConfigData(sLangID);
				
			sChildKeys = sSettings.childKeys();
			foreach (const QString &childKey, sChildKeys)
			{
				if(INI_LANGUAGE_STATE_SELECTING_IMG == childKey)		
				{
					pNew->m_sLangSelectingIMG = sSettings.value(childKey).toString();
				}
				else if(INI_LANGUAGE_STATE_SELECTED_IMG == childKey)	
				{
					pNew->m_sLangSelectedIMG = sSettings.value(childKey).toString();
				}
				else if(INI_LANGUAGE_LANG_SHORT_LABEL == childKey)	
				{
					pNew->m_sLangShortLabel = sSettings.value(childKey).toString();;
				}

			}
			
			m_lstLangPickCfg.append(pNew);

		sSettings.endGroup();
	}

	//---------------------------------------------------------------
	return true;
}

/*-------------------------------- END OF FILE ------------------------------*/