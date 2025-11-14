#ifndef MIHM_CONFIG_GENERAL_H
#define MIHM_CONFIG_GENERAL_H

#include "MIhmLaneTypeSettings.h"
#include "MIhmConfigColor.h"
#include "MIhmLanguages.h"
#include "MIhmConfigImages.h"
#include "MIhmConfigActions.h"
#include "MIhmConfigCursors.h"
#include "MIhmConfigDynamic.h"

#include "MPoliceValue.h"


#include <QObject>
#include <QList>
#include <QString>
#include <QFont>
#include <QRect>
#include <QThread>


extern "C" {
	#include <noyau.h>
	#include "ihm_loc.h"

}


//------------------------------------------------------------------
//General configuration data container class
//The 	read from registry
//------------------------------------------------------------------
class MIhmConfigGeneral 
{
	
	friend class MIhmAniInitData;
	
	public:
		MIhmConfigGeneral();
		~MIhmConfigGeneral();
		
		QString getModuleMboxName(){return m_sModuleMboxName;};
		QString getModuleConfigKey(){return m_sModuleConfigKey;};

		static bool getPoliceFontValue(MPoliceValue * pPoliceValue, QFont *pFnt);
		static QString getPoliceFontStyle(MPoliceValue * pPoliceValue);

		//used to initialize all configuration files at the app start
		bool loadConfigFromRegistry(QString sMboxName); 

		//used to dynamically reload the DynamicConfig.xml file
		bool reloadConfigDynamic();

		//bool loadConfigFromFile(QString sConfigFilenameAndPath);

		QString getCSSFullPath(QString sUI);
		QString getUIFullPath(QString sUI);
		QString getUIInputExDefPath(QString sFileName);
		QString getCommFileFullPath(QString sDataFile);

		MIhmConfigColor * getConfigColors(){return &m_oConfigColors;};

		QString	m_sDefaultFont;
		DWORD	m_dwDefaultFontColour;



		bool	m_bDisplayEntryGate;
		bool	m_bDisplayEntryLoop;

		bool	m_bHideCursor;
		DWORD	m_dwPriorityClass;

		bool	m_bInvalidKeyDisplay;

		bool	m_bShowTitleBar;

		DWORD	m_dwStartTimeout;
		DWORD	m_dwStopTimeout;

		
		bool	m_bShowSplash;
		QString m_sSplashTemplate;
		QString m_sDefaultInputTemplate;
		QString m_sAboutDlgTemplate;
		QString m_sAboutDlgDefaultCfg;

		QString m_sSystemEncoding;
		
		bool isWebEnabled(){return m_bWebInterfaceEnabled;};
		
		QThread::Priority getWebPriority(){return m_eWebThreadPriority;};
		QThread::Priority getAniPriority(){ return m_eAniThreadPriority;};

		MIhmLaneTypeSettings * getLaneTypeSetting(MIhmLaneTypeSettings::enumSettingsType eType,
												  QString sLaneType);

		QList <MIhmLaneTypeSettings *> m_lstLaneTypeSettings;

		MIhmLanguages * getLanguages(){return &m_oLanguages;};
		MIhmConfigImages * getDskConfigImages() {return &m_oConfImages;};
		MIhmConfigActions * getActions() {return &m_oConfigActions;};
		MIhmConfigDynamic * getCfgDynamic() {return &m_oConfigDynamic;};
		
		QCursor getCursor(QString sCursorId);

		int getTabFirstIndex(QString sTabName);

		static MIhmConfigGeneral * getCfg(){return m_pGenCfg;};


		QString getDefaultLoginPrefix(){return m_sDefaultLoginPrefix;};

		QChar getDefaultDecimalSeparator(){ return m_cDefaultDecimalSeparator;};
		int getDefaultResultPrecision(){return m_iDefaultResultPrecision;};
		
		QString getDefaultTimeFormat(){return m_sDefaultTimeFormat;};
		QString getDefaultDateFormat(){return m_sDefaultDateFormat;};

		int getAboutKey(){return m_dwAboutKey;};
		int getRefreshKey(){return m_dwRefreshKey;};
		
		int getReturnControlKey(){ return m_dwReturnControlKey; };
		int getTakeControlKey(){ return m_dwTakeControlKey; };
		
		int getNumLockSynchroKey(){return m_dwNumLockSynchroKey;};
		int getNumLockPeriod(){return m_dwNumLockPeriod;};

		int getValidKey(){return m_dwValidKey;};
		int getMenuKey(){return m_dwMenuKey;};

		bool isManualStartupPosition(){return m_bManualStartupPosition;};
		int getStartupPositionLeft(){return m_dwStartupPositionLeft;};
		int getStartupPositionTop(){return m_dwStartupPositionTop;};
		int getStartupScreenIndex(){ return m_dwStartupScreenIndex; };
		bool getStartupHideIfSingleScreen(){ return m_bStartupHideIfSingleScreen; };

		int getResolutionX(){return m_dwResolutionX;};
		int getResolutionY(){return m_dwResolutionY;};
		bool isNoResolutionCheck(){return m_bNoResolutionCheck;};

		int getAuthReqTimeout(){return m_iAuthReqTimeout;};

		bool getVideoZoomParams(int iIndex, QRect &rctZoomParams);
		QString createDefaultVideoFullPath(QString sImageName);
		QString getDefaultVideoPicture() { return m_sDefaultVideoPicture;};
		DWORD	getDefaultVideoSource() { return m_dwDefaultVideoSource;};
		DWORD	getDefaultVideoZoom() { return m_dwDefaultVideoZoom;};
		DWORD	getVideoZoomNo() { return m_dwVideoZoomNo;};
		bool	doUITemplatesVerification(){return m_bDoUITemplatesVerification;};

		bool getInputDlgsValidAlwaysEnabled() { return m_bInputDlgsValidAlwaysEnabled;};
		QString getInputDlgsErrorFieldsStyle(){ return m_sInputDlgsErrorFieldsStyle;};
		
		bool getShowCancelOnConnectingDlg(){return m_bShowCancelOnConnectingDlg;};
		DWORD getMaxInputMsgPerSec(){ return m_dwMaxInputMsgPerSec;}

		static QString getCleanPath(QString sPath, QString sSubPath);

		QString getStrDetMaskRegex(){ return m_sTestKeyboardStrDetMaskRegex;};
		bool getStrDetMaskEnabled() { return (m_dwTestKeyboardMaskEnabled!=0)?true:false;};
		QChar getStrDetMaskChar(){ return m_chTestKeyboardMaskChar;};
		bool getFilterStringDetectedKeys() {return m_bFilterStringDetectedKeys;};

		//virtual keyboard parameters...
		bool isVirtualKeyboardEnabled() {return m_bVirtualKeyboardEnabled;};
		QString getVirtualKeyboardMboxName() {return m_sVirtKeybMboxName;};


	private:
		QThread::Priority m_eAniThreadPriority; 
		QThread::Priority m_eWebThreadPriority; 
		
		bool	m_bDoUITemplatesVerification;
		bool	m_bNoResolutionCheck;
		DWORD	m_dwResolutionX;
		DWORD	m_dwResolutionY;
		bool	m_bManualStartupPosition;
		DWORD	m_dwStartupPositionLeft;
		DWORD	m_dwStartupPositionTop;
		DWORD   m_dwStartupScreenIndex;
		bool   m_bStartupHideIfSingleScreen;
			
		bool parseDefaultFontConfig(QString sFont); 
		bool loadAvailableLaneTypesDsk(QString sLaneTypesCfgFilePath);

		static MIhmConfigGeneral * m_pGenCfg;

		QString m_sDefaultDateFormat;		
		QString m_sDefaultTimeFormat;	
		
		QChar m_cDefaultDecimalSeparator;
		int m_iDefaultResultPrecision;
		QString m_sInputDlgsErrorFieldsStyle;
		bool m_bInputDlgsValidAlwaysEnabled;

		bool m_bShowCancelOnConnectingDlg;

		DWORD m_iAuthReqTimeout;

		//global keys
		DWORD	m_dwAboutKey;
		DWORD	m_dwMenuKey; 
		DWORD	m_dwRefreshKey;
		DWORD	m_dwReturnControlKey;
		DWORD	m_dwTakeControlKey;
		
		DWORD	m_dwValidKey;

		DWORD	m_dwNumLockPeriod;
		DWORD	m_dwNumLockSynchroKey;


		QString m_sDefaultLanguage;
		QString m_sLanguagesCfgFilePath;
		MIhmLanguages		m_oLanguages;

		QString m_sImagesCfgFilePath;
		MIhmConfigImages	m_oConfImages;

		QString m_sCursorsCfgFilePath;
		MIhmConfigCursors	m_oConfigCursors;

		QString				m_sDynamicColorsFile;
		MIhmConfigColor		m_oConfigColors;
		
		QString m_sActionsCfgFilePath;
		MIhmConfigActions	m_oConfigActions;

		QString m_sDynamicCfgFilePath;
		MIhmConfigDynamic	m_oConfigDynamic;

		QString m_sLaneTypesCfgFile;

		QString m_sModuleMboxName;
		QString m_sModuleConfigKey;	

		QString m_sDefaultLoginPrefix;

		QString m_sUICssRoot;
		QString m_sUITemplateRoot;
		QString m_sUIInputExDefRoot;
		QString	m_sCommFilesRoot;
		
		//WEB specific
		bool	m_bWebInterfaceEnabled;

		//Video params
		QString m_defaultVideoFullPath;
		QString m_sDefaultVideoDir;
		QString m_sDefaultVideoPicture;
		DWORD	m_dwDefaultVideoSource;
		DWORD	m_dwDefaultVideoZoom;
		DWORD	m_dwVideoZoomNo;
		QList <QRect> m_lstVideoZoomParams;

		bool addVideoZoomParams(QString sVideoZoomStr);

		//Used to limit frequency of input messages emit from desktop interface
		//to solve the problem of system jamed with input 
		DWORD m_dwMaxInputMsgPerSec;

		QString		m_sTestKeyboardStrDetMaskRegex;
		DWORD		m_dwTestKeyboardMaskEnabled;
		QChar		m_chTestKeyboardMaskChar;
		bool		m_bFilterStringDetectedKeys;

		//virtual keyboard parameters...
		bool m_bVirtualKeyboardEnabled;
		QString m_sVirtKeybMboxName;
};






#endif


