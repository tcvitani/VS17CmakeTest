/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:     MLangPickConfig.h												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef COMMON_TOOLS_LANG_PICK_CONFIG_H
#define COMMON_TOOLS_LANG_PICK_CONFIG_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QObject>
#include <QList>
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


class COMMON_TOOLS_CLASSES_DLLSPEC MLangPickItemConfigData
{
	public:
		MLangPickItemConfigData(QString sLangId):m_sLangId(sLangId){};
		
	QString m_sLangId;
	QString m_sLangSelectingIMG;
	QString m_sLangSelectedIMG;
	QString m_sLangShortLabel;
};




class COMMON_TOOLS_CLASSES_DLLSPEC MLangPickConfig
{	
	public:
		MLangPickConfig(QString s){m_sConfigFilePath = s;};
		virtual ~MLangPickConfig();
		
		//used to load settings for language pick control
		bool loadConfig();

		QList <MLangPickItemConfigData*> * getLangPickCfgList(){return &m_lstLangPickCfg;};

	private:
		bool loadLanguageConfig();
		QString	m_sConfigFilePath;

		QList <MLangPickItemConfigData *> m_lstLangPickCfg;
		
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/