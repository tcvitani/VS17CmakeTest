/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:     MConfig.h														 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef COMMON_TOOLS_CONFIG_BASE_H
#define COMMON_TOOLS_CONFIG_BASE_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QString>

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/


#if defined COMMON_TOOLS_CLASSES_EXPORT
	#define COMMON_TOOLS_CLASSES_DLLSPEC  Q_DECL_EXPORT
#else
	#define COMMON_TOOLS_CLASSES_DLLSPEC Q_DECL_IMPORT
#endif


/*-------------------------------- TYPEDEFS:  -------------------------------*/

class MTranslator;

class COMMON_TOOLS_CLASSES_DLLSPEC MConfig 
{	
	public:
		MConfig(QString sAppBaseConfigKeyName);
		virtual ~MConfig();
		static MConfig * getCfg(){return m_pCfg;}

		QString getConfigFilePath();
		
		//used to initialize all configuration files at the app start
		bool loadConfig();
		virtual void parseCmdlineArgs();

		MTranslator* getTranslator(){ return m_pTranslator;	}
		QString getLangFilePath() { return m_LangFilePath;}
		QString getDefaultLanguageId() { return m_sDefaultLanguage;}
		
		QString getWindowIconPath(){ return m_WindowIconPath; }

		QString getFilePrefix(){ return m_FilePrefix; }

		QString getTrcFilePath(){ return m_TrcFilePath; }
		QString getErrFilePath(){ return m_ErrFilePath; }

		QString getStylesheetPath(){ return m_StylesheetPath; }
		QString getDialogStylesheetPath(){ return m_DialogStylesheetPath; }

		int getTraceMaxMB(){ return m_iTraceMaxMb; }
		
		bool isTrcEnabled(){ return m_bIsTrcEnabled; }
		bool isErrEnabled(){ return m_bIsErrEnabled; }

		//about dlg ...
		QString getLogoPixPath(){return m_sLogoPixPath;}
		QString getVirtKeybPath(){return m_sVirtKeybPath;}

		QString getWindowSize() { return m_sWindowSize;}
		QString getWindowPosition() { return m_sWindowPosition;}
		
	private:
		virtual QString getDefaultConfigFilePath() = 0;
		bool loadLanguageConfig();
		QString m_sConfigFilePath;
	
		QString			m_FilePrefix;
		QString			m_TrcFilePath;
		QString			m_ErrFilePath;
		QString			m_StylesheetPath;
		QString			m_DialogStylesheetPath;
		QString			m_WindowIconPath;
		QString			m_LangFilePath;
		QString			m_sDefaultLanguage;
		QString			m_sCommandLineLanguage;
		QString			m_sVirtKeybPath;
		QString			m_sWindowSize;
		QString			m_sWindowPosition;

		const QString	m_sAppBaseConfigKeyName;

		//about dlg ...
		QString			m_sLogoPixPath;

		int				m_iTraceMaxMb;

		bool			m_bIsTrcEnabled;
		bool			m_bIsErrEnabled;

		MTranslator		*m_pTranslator;
		static MConfig * m_pCfg;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/