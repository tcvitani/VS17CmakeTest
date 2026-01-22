/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:     MConfig.cpp													 */
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

#include "MTranslator.h"

#include "MConfig.h"
#include "MHelpFuncs.h"
#include "MTracer.h"

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/
#define CONFIG_INI_LANG_FILE_PATH			"LanguageFilePath"
#define CONFIG_INI_LOGO_FILE_PATH			"AboutDialogLogoPath"
#define CONFIG_INI_DEFAULT_LANG				"DefaultLanguage"
#define CONFIG_INI_TRACE_FILE_PATH			"TracePath"
#define CONFIG_INI_ERROR_FILE_PATH			"ErrorPath"
#define CONFIG_INI_WINDOW_ICON_PATH			"WindowIconPath"
#define CONFIG_INI_STYLESHEET_PATH			"StylesheetPath"
#define CONFIG_INI_DIALOG_STYLESHEET_PATH	"DialogStylesheetPath"
#define CONFIG_INI_FILE_MAX_SIZE			"FileMaxSize"
#define CONFIG_INI_TRACE_FILE_PREFIX		"TraceAndErrorFileNamePrefix"
#define CONFIG_INI_TRACE_FILE_ENABLED		"TraceFileEnabled"
#define CONFIG_INI_ERROR_FILE_ENABLED		"ErrorFileEnabled"
#define CONFIG_INI_VIRTUAL_KEYBOARD_PATH	"VirtualKeyboardPath"
#define CONFIG_INI_WINDOW_SIZE				"WindowSize"
#define CONFIG_INI_WINDOW_POSITION			"WindowPosition"


/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/
MConfig * MConfig::m_pCfg = NULL;
MConfig::MConfig(QString sAppBaseConfigKeyName):
	m_iTraceMaxMb(0),m_bIsTrcEnabled(false),m_bIsErrEnabled(false),
	m_sAppBaseConfigKeyName(sAppBaseConfigKeyName),
	m_sVirtKeybPath("C:\\Windows\\System32\\osk.exe")
{
	if(m_pCfg!=NULL)
		qFatal("MConfig class instance already created!");

	m_pCfg = this;
}

MConfig::~MConfig()
{
	m_pTranslator = NULL;
}


QString MConfig::getConfigFilePath()
{
	if(m_sConfigFilePath.isNull() || m_sConfigFilePath.isEmpty())
	{
		m_sConfigFilePath = getDefaultConfigFilePath();
	}
	return m_sConfigFilePath;
}

void MConfig::parseCmdlineArgs()
{
	enum EnumCurrOpt {OPT_NONE, OPT_LANG, OPT_CONFIG};
	EnumCurrOpt currOpt = OPT_NONE;
	QStringList cmdline_args = QCoreApplication::arguments();

	QStringList::const_iterator constIterator;
	for (constIterator = cmdline_args.constBegin(); constIterator != cmdline_args.constEnd();++constIterator)
	{
		QString currValue = (*constIterator);

		if(currOpt == OPT_LANG)
		{
			m_sCommandLineLanguage = currValue;
			currOpt = OPT_NONE;
		}
		else if(currOpt == OPT_CONFIG)
		{
			m_sConfigFilePath = currValue;
			currOpt = OPT_NONE;
		}
		else if(currOpt == OPT_NONE)
		{
			if(currValue == "-l" || currValue == "--lang")
				currOpt = OPT_LANG;
			else if(currValue == "-c" || currValue == "--config")
				currOpt = OPT_CONFIG;
			else if(constIterator != cmdline_args.constBegin())
				TRACE_W(QString("Unknown command line parameter %1")
					.arg(currValue));
			
		}
	}
}


bool MConfig::loadConfig()
{
	QString			sValue;
	QStringList		sChildKeys;	
	QString			sLangFilePath;
	QString			sLanguage;

	TRACE_D("MConfig::loadConfig()" );

	///////////////////////////////////////////////////////////////////////////
	m_sConfigFilePath = getConfigFilePath();

	// CHECK THAT .INI FILE EXIST
	QFile checkConfig(m_sConfigFilePath);
	if (!checkConfig.exists())
	{			
		TRACE_W(QString("main() => Error loading config file: %1")
			.arg(m_sConfigFilePath));
		
		QMessageBox::critical(NULL, 
			MHelpFuncs::getProcessFileName(),
			QString("Error loading config file: %1").arg(m_sConfigFilePath));
		
		return false;
	}
	
	// GET .INI FILE
	QSettings sSettings (m_sConfigFilePath, QSettings::IniFormat);
	sSettings.sync();

	sSettings.beginGroup(m_sAppBaseConfigKeyName);
	
	// GET TRACE FILE PATH FROM .INI FILE
	sValue.clear();
	sValue = sSettings.value(CONFIG_INI_TRACE_FILE_PATH, "").toString();	
	if (sValue.isEmpty() || sValue.isNull())
	{	
		TRACE_W(QString("MConfig::loadConfig() => Error loading [%1] from .ini file: %2 group:%3")
			.arg(CONFIG_INI_TRACE_FILE_PATH)
			.arg(m_sConfigFilePath)
			.arg(m_sAppBaseConfigKeyName));
				
		return false;
	}
	m_TrcFilePath = sValue;

	// GET ERROR FILE PATH FROM .INI FILE
	sValue.clear();
	sValue = sSettings.value(CONFIG_INI_ERROR_FILE_PATH, "").toString();	
	if (sValue.isEmpty() || sValue.isNull())
	{	
		TRACE_W(QString("MConfig::loadConfig() => Error loading [%1] from .ini file: %2 group:%3")
			.arg(CONFIG_INI_ERROR_FILE_PATH)
			.arg(m_sConfigFilePath)
			.arg(m_sAppBaseConfigKeyName));
		return false;
	}
	m_ErrFilePath = sValue;

	// GET FILE MAX SIZE FROM .INI FILE
	m_iTraceMaxMb = sSettings.value(CONFIG_INI_FILE_MAX_SIZE, "").toInt();	
	if (m_iTraceMaxMb <= 0)
	{	
		TRACE_W(QString("MConfig::loadConfig() => Error loading [%1] from .ini file: %2 group:%3")
			.arg(CONFIG_INI_FILE_MAX_SIZE)
			.arg(m_sConfigFilePath)
			.arg(m_sAppBaseConfigKeyName));
		
		return false;
	}
	// GET TRACE AND ERROR FILE PREFIX FROM .INI FILE
	sValue.clear();
	sValue = sSettings.value(CONFIG_INI_TRACE_FILE_PREFIX, MHelpFuncs::getProcessFileName()).toString();	
	if (sValue.isEmpty() || sValue.isNull())
	{	
		TRACE_W(QString("MConfig::loadConfig() => Error loading [%1] from .ini file: %2 group:%3")
			.arg(CONFIG_INI_TRACE_FILE_PREFIX)
			.arg(m_sConfigFilePath)
			.arg(m_sAppBaseConfigKeyName));
		
		return false;
	}
	m_FilePrefix = sValue;
	
	// CHECK IS TRACE FILE ENABLED
	m_bIsTrcEnabled = sSettings.value(CONFIG_INI_TRACE_FILE_ENABLED, "true").toBool();	
	
	// CHECK IS ERROR FILE ENABLED
	m_bIsErrEnabled = sSettings.value(CONFIG_INI_ERROR_FILE_ENABLED, "true").toBool();	

	// GET LANGUAGE FILE PATH FROM INI FILE
	sLangFilePath = sSettings.value(CONFIG_INI_LANG_FILE_PATH, "").toString();	
	if (sLangFilePath.isEmpty() || sLangFilePath.isNull())
	{	
		TRACE_W(QString("main() => Error loading lang. file path from .ini file: %1 group:%2")
			.arg(m_sConfigFilePath).arg(m_sAppBaseConfigKeyName));
		
		QMessageBox::critical(NULL, 
			MHelpFuncs::getProcessFileName(),
			QString("Error loading lang. file path from .ini file: %1 group:%2")
			.arg(m_sConfigFilePath).arg(m_sAppBaseConfigKeyName));
		
		
		return false;
	}

	// CHECK THAT LANGUAGE FILE EXIST
	QFile checkLangFile(sLangFilePath);
	if (!checkLangFile.exists())
	{	
		TRACE_W(QString("main() => Error loading language file: %1")
			.arg(sLangFilePath));
		
		QMessageBox::critical(NULL, 
			MHelpFuncs::getProcessFileName(),
			QString("Error loading language file: %1")
			.arg(sLangFilePath));
		
		return false;
	}
	m_LangFilePath = sLangFilePath;
	
	// GET DEFAULT LANGUAGE FROM INI FILE		
	sLanguage = sSettings.value(CONFIG_INI_DEFAULT_LANG, "").toString();	
	if (sLanguage.isEmpty() || sLanguage.isNull())
	{	
		TRACE_W(QString("main() => Error loading language from .ini file: .ini file: %1 group:%2")
			.arg(m_sConfigFilePath).arg(m_sAppBaseConfigKeyName));
		
		
		QMessageBox::critical(NULL, 
			MHelpFuncs::getProcessFileName(),
			QString("Error loading language from .ini file: .ini file: %1 group:%2")
			.arg(m_sConfigFilePath)
			.arg(m_sAppBaseConfigKeyName));
		
		
		return false;
	}
	
	if(m_sCommandLineLanguage.isEmpty()) 
		m_sDefaultLanguage = sLanguage; //Set only if not already set from command line
	else
		m_sDefaultLanguage = m_sCommandLineLanguage;


	// GET STYLESHEET PATH FROM .INI FILE
	sValue.clear();
	sValue = sSettings.value(CONFIG_INI_STYLESHEET_PATH, "").toString();	
	if (!sValue.isEmpty() && !sValue.isNull())
	{	
		m_StylesheetPath = sValue;
	}
	
	// GET STYLESHEET PATH FOR MODAL DIALOGS WINDOW FROM .INI FILE
	sValue.clear();
	sValue = sSettings.value(CONFIG_INI_DIALOG_STYLESHEET_PATH, "").toString();	
	if (!sValue.isEmpty() && !sValue.isNull())
	{	
		m_DialogStylesheetPath = sValue;
	}
	else
		m_DialogStylesheetPath = QString();

	// GET WINDOW ICON PATH FROM .INI FILE
	sValue.clear();
	sValue = sSettings.value(CONFIG_INI_WINDOW_ICON_PATH, "").toString();	
	if (sValue.isEmpty() || sValue.isNull())
	{	
		TRACE_W(QString("MConfig::loadConfig() => Error loading [%1] from .ini file: %2 group:%3")
			.arg(CONFIG_INI_WINDOW_ICON_PATH)
			.arg(m_sConfigFilePath)
			.arg(m_sAppBaseConfigKeyName));
		
		return false;
	}
	m_WindowIconPath = sValue;


	sValue.clear();
	sValue = sSettings.value(CONFIG_INI_LOGO_FILE_PATH, "").toString();	
	if (sValue.isEmpty() || sValue.isNull())
	{	
		TRACE_W(QString("MConfig::loadConfig() => Error loading [%1] from .ini file: %2 group:%3")
			.arg(CONFIG_INI_LOGO_FILE_PATH)
			.arg(m_sConfigFilePath)
			.arg(m_sAppBaseConfigKeyName));
		
		return false;
	}
	m_sLogoPixPath = sValue;
	
	sValue.clear();
	sValue = sSettings.value(CONFIG_INI_VIRTUAL_KEYBOARD_PATH, "").toString();	
	if (sValue.isEmpty() || sValue.isNull())
	{	
		TRACE_W(QString("MConfig::loadConfig() => Error loading [%1] from .ini file: %2 group:%3")
			.arg(CONFIG_INI_VIRTUAL_KEYBOARD_PATH)
			.arg(m_sConfigFilePath)
			.arg(m_sAppBaseConfigKeyName));
	}

	m_sVirtKeybPath = sValue;

	// GET INITIAL WINDOW SIZE FROM .INI FILE
	sValue.clear();
	sValue = sSettings.value(CONFIG_INI_WINDOW_SIZE, "").toString();	
	if (!sValue.isEmpty() && !sValue.isNull())
	{	
		m_sWindowSize = sValue;
	}
	else
		m_sWindowSize.clear();

	// GET WINDOW POSITION FROM .INI FILE
	sValue.clear();
	sValue = sSettings.value(CONFIG_INI_WINDOW_POSITION, "").toString();	
	if (!sValue.isEmpty() && !sValue.isNull())
	{	
		m_sWindowPosition = sValue;
	}
	else
		m_sWindowPosition.clear();

	sSettings.endGroup();

	//---------------------------------------------------------------


	//---------------------------------------------------------------

	m_pTranslator = new MTranslator();

	if( m_pTranslator->initLanguagesFromFile( getLangFilePath(), getDefaultLanguageId()) != MTRANS_SUCCESS) 
	{
		TRACE_W(QString("main() => Error initializing language %1  from file: %2")
				.arg(getDefaultLanguageId()).arg(getLangFilePath()));
		return false;
	}
	else
	{
		qApp->installTranslator(m_pTranslator);
	}

	return true;
}

/*-------------------------------- END OF FILE ------------------------------*/