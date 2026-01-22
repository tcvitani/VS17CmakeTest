/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 CmdSvc_tool												 */
/* FILE:     MSpyToolConfig.h											 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMDSVC_TOOL_CONFIG_H
#define CMDSVC_TOOL_CONFIG_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QObject>
#include <QList>
#include <QString>
#include <QFont>
#include <QRect>
#include <QStringList>
#include <MTranslator.h>

#include "MDefines.h"
#include "MGlobalStructs.h"
#include "MHelpFuncs.h"
#include "MConfig.h"

extern "C"
{


};

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

class MLangPickConfig;

/*-------------------------------- DEFINES:   -------------------------------*/

#define CMDSVC_TOOL_LABEL_ERR_TITLE		"CMDSVC TOOL"
#define TOOL_GEN_SETTINGS_INI_FILE_KEY	"GENERAL_SETTINGS"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

class MCmdSvcToolConfig :public MConfig
{	
	public:
		MCmdSvcToolConfig(QString sAppBaseConfigKeyName);
		virtual ~MCmdSvcToolConfig();
		static MCmdSvcToolConfig * getCfg(){ return (MCmdSvcToolConfig *)MConfig::getCfg(); };

		bool loadConfig();

		virtual QString getDefaultConfigFilePath();

		MLangPickConfig * getLangPickConfig() { return m_pLangPickCfg; };
		
	private:
		MLangPickConfig		*m_pLangPickCfg;
		QString				m_sInstanceName;
		QString				m_sFormatPath;

		//-------------------------------

};

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/