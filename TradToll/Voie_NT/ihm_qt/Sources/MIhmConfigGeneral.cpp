

#include "MIhmConfigGeneral.h"
#include "MHelpFuncs.h"
#include "MTracer.h"

#include <QCoreApplication>

#include <QFont>
#include <QStringList>
#include <QDir>
#include <QColor>

extern "C" {
	#include <ihm.H>
	#include <reg.h>
	#include <noyau.h>
};

MIhmConfigGeneral * MIhmConfigGeneral::m_pGenCfg = NULL;



MIhmConfigGeneral::MIhmConfigGeneral()
{
	m_sModuleMboxName = "";
	m_sModuleConfigKey = "";
	m_eAniThreadPriority = QThread::NormalPriority; 
	m_eWebThreadPriority = QThread::NormalPriority; 

}


MIhmConfigGeneral::~MIhmConfigGeneral()
{
	while(!m_lstLaneTypeSettings.isEmpty())
		delete m_lstLaneTypeSettings.takeLast();
	
}



bool MIhmConfigGeneral::loadConfigFromRegistry(QString sMboxName)
{
    DWORD dwValeurLen;
    DWORD dwRes;
    DWORD dwTemp;
    char szTemp[IHM_LG_LIGNE_MAX];
    char pcRegKey[IHM_LG_LIGNE_MAX];

	//make this ConfigGeneral object public by assigning to the public static variable
	m_pGenCfg = this;
	
	m_sModuleMboxName = sMboxName;

    // module config key
    m_sModuleConfigKey = QString("%1%2%3%4%5%6").arg(CSR_REG_KEYn_CSRBASE)
								.arg(CSR_REG_KEYn_LANE_BASE)
								.arg(CSR_REG_KEYn_CONFIG)
								.arg(MOD_REG_KEYn_MODULES)
								.arg(IHM_REG_KEYn_ModIHM)
								.arg(m_sModuleMboxName);

	
	strcpy_s(pcRegKey, sizeof(pcRegKey)-1, m_sModuleConfigKey.toLatin1().data());

    TRACE_D("MIhmConfigGeneral::loadConfigFromRegistry..." );

	// 	m_sDefaultFont;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_DEFAULT_FONT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString("MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg(IHM_REG_VAL_DEFAULT_FONT) );
		return false;
	}
	m_sDefaultFont = szTemp;

	if(!parseDefaultFontConfig(m_sDefaultFont))
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error loading default font: %1").arg(szTemp));
		return false;
	}

	// 	m_sDefaultDateFormat;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_DEFAULT_DATE_FORMAT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg(IHM_REG_VAL_DEFAULT_DATE_FORMAT ));
		return false;
	}

	m_sDefaultDateFormat = szTemp;

	// 	m_sDefaultTimeFormat;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_DEFAULT_TIME_FORMAT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg(IHM_REG_VAL_DEFAULT_TIME_FORMAT));
		return false;
	}

	m_sDefaultTimeFormat = szTemp;

	


	// 		QString m_dwDefaultVideoPicture;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_DEFAULT_VIDEO_PIC,
		szTemp, 
		&dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg(IHM_REG_VAL_DEFAULT_VIDEO_PIC));
		return false;
	}
	m_sDefaultVideoPicture = szTemp;

	// 		DWORD	m_dwDefaultVideoSource;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_DEFAULT_VIDEO_SOURCE,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg(IHM_REG_VAL_DEFAULT_VIDEO_SOURCE));
		return false;
	}
	m_dwDefaultVideoSource = dwTemp;

	// 		DWORD	m_dwDefaultVideoZoom;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_DEFAULT_VIDEO_ZOOM,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg(IHM_REG_VAL_DEFAULT_VIDEO_ZOOM));
		return false;
	}
	m_dwDefaultVideoZoom = dwTemp;
	//		DWORD	m_dwVideoZoomNo;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_VIDEO_ZOOM_NO,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_VIDEO_ZOOM_NO));
		return false;
	}
	m_dwVideoZoomNo = dwTemp;
	//#define IHM_REG_VAL_VIDEO_ZOOM_X			"VideoZoom"
	//		QStringList m_lstVideoZoom;
	for(unsigned int i = 0; i < m_dwVideoZoomNo; i++)
	{
		dwValeurLen = sizeof (szTemp);
		if ((dwRes = REG_Lire_Chaine (
			CSR_REG_KEYi_ROOT, 
			pcRegKey,
			(char*)(QString("%1%2").arg(IHM_REG_VAL_VIDEO_ZOOM_X).arg(i)).toLatin1().data(),
			szTemp, 
			&dwValeurLen) ) != ERROR_SUCCESS)
		{
			TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2%3]")
						.arg(pcRegKey).arg(IHM_REG_VAL_VIDEO_ZOOM_X).arg(i));
			return false;
		}

		if(!addVideoZoomParams(szTemp))
		{
			TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error parsing zoom parameters for key %1[%2%3]!")
						.arg(pcRegKey).arg(IHM_REG_VAL_VIDEO_ZOOM_X).arg(i));
			return false;
		}
	}
	

// 		bool	m_bHideCursor;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_HIDE_CURSOR, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_HIDE_CURSOR));
		return false;
	}

	m_bHideCursor = (dwTemp==0)?false:true;


// 		bool	m_bInvalidKeyDisplay;

	
// 		bool	m_bNoResolutionCheck;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_NO_RESOLUTION_CHECK, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_NO_RESOLUTION_CHECK));
		m_bNoResolutionCheck = false;
	}
	else
		m_bNoResolutionCheck = (dwTemp==0)?false:true;



	//m_bDoUITemplatesVerification
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_DO_UI_TEMPL_VERIFICATION, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]. Setting default to false!")
						.arg(pcRegKey).arg(IHM_REG_VAL_DO_UI_TEMPL_VERIFICATION));
		m_bDoUITemplatesVerification = false;
	}
	else
		m_bDoUITemplatesVerification = (dwTemp==0)?false:true;


	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_ABOUT_KEY, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_ABOUT_KEY));
		return false;
	}
	
	m_dwAboutKey = dwTemp;

// 		DWORD	m_dwMenuKey;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_MENU_KEY, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_MENU_KEY));
		return false;
	}
	
	m_dwMenuKey = dwTemp;




// 		DWORD	m_dwRefreshKey;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_REFRESH_KEY, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_REFRESH_KEY));
		return false;
	}
	
	m_dwRefreshKey = dwTemp;


// 		DWORD	m_dwReturnControlKey;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_RETURNCTRL_KEY, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_RETURNCTRL_KEY));
		return false;
	}
	
	m_dwReturnControlKey = dwTemp;

	
	// 		DWORD	m_dwTakeControlKey;
	if ((dwRes = REG_Lire_Entier(
		CSR_REG_KEYi_ROOT,
		pcRegKey,
		(char*)IHM_REG_VAL_TAKECTRL_KEY,
		&dwTemp)) != ERROR_SUCCESS)
	{
		TRACE_W(QString("MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg(IHM_REG_VAL_TAKECTRL_KEY));
		m_dwTakeControlKey = 275;
	}
	else
		m_dwTakeControlKey = dwTemp;

// 		DWORD	m_dwValidKey;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_VALID_KEY, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_VALID_KEY));
		return false;
	}
	
	m_dwValidKey = dwTemp;


// 		DWORD	m_dwResolutionX;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_RESOLUTION_X, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_RESOLUTION_X));
		return false;
	}
	
	m_dwResolutionX = dwTemp;

// 		DWORD	m_dwResolutionY;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_RESOLUTION_Y, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_RESOLUTION_Y));
		return false;
	}
	
	m_dwResolutionY = dwTemp;


// 		bool	m_bManualStartupPosition;
		if ((dwRes = REG_Lire_Entier (
							CSR_REG_KEYi_ROOT, 
							pcRegKey,
							(char*)IHM_REG_VAL_MANUAL_STARTUP_POSITION, 
							&dwTemp)) != ERROR_SUCCESS)
		{
			TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_MANUAL_STARTUP_POSITION));
			m_bManualStartupPosition = true;
		}
		else
			m_bManualStartupPosition = (dwTemp==0)?false:true;

// 		DWORD	m_dwStartupPositionLeft;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_STARTUP_POSITION_LEFT, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_STARTUP_POSITION_LEFT));
		return false;
	}

	m_dwStartupPositionLeft = dwTemp;

// 		DWORD	m_dwStartupPositionTop;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_STARTUP_POSITION_TOP, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_STARTUP_POSITION_TOP));
		return false;
	}

	m_dwStartupPositionTop = dwTemp;

	
	// 		DWORD	m_dwStartupScreenIndex;
		if ((dwRes = REG_Lire_Entier(
			CSR_REG_KEYi_ROOT,
			pcRegKey,
			(char*)IHM_REG_VAL_STARTUP_SCREEN_INDEX,
			&dwTemp)) != ERROR_SUCCESS)
		{
			m_dwStartupScreenIndex = 0;
			TRACE_W(QString("MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! Using default:%3")
				.arg(pcRegKey).arg(IHM_REG_VAL_STARTUP_SCREEN_INDEX).arg(m_dwStartupScreenIndex));
		}
		else
			m_dwStartupScreenIndex = dwTemp;

		
		// 		DWORD	m_bStartupHideIfSingleScreen;
		if ((dwRes = REG_Lire_Entier(
			CSR_REG_KEYi_ROOT,
			pcRegKey,
			(char*)IHM_REG_VAL_STARTUP_HIDE_IF_SINGLE_SCREEN,
			&dwTemp)) != ERROR_SUCCESS)
		{
			m_bStartupHideIfSingleScreen = false;
			TRACE_W(QString("MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! Using default:%3")
				.arg(pcRegKey).arg(IHM_REG_VAL_STARTUP_HIDE_IF_SINGLE_SCREEN).arg(m_bStartupHideIfSingleScreen?1:0));
		}
		else
			m_bStartupHideIfSingleScreen = dwTemp==0?false:true;


// 		bool	m_bShowTitleBar;
		if ((dwRes = REG_Lire_Entier (
							CSR_REG_KEYi_ROOT, 
							pcRegKey,
							(char*)IHM_REG_VAL_SHOW_TITLE_BAR, 
							&dwTemp)) != ERROR_SUCCESS)
		{
			TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_SHOW_TITLE_BAR));
			m_bShowTitleBar = true;
		}
		else
			m_bShowTitleBar = (dwTemp==0)?false:true;

		
// 		DWORD	m_dwStartTimeout;
		if ((dwRes = REG_Lire_Entier (
							CSR_REG_KEYi_ROOT, 
							pcRegKey,
							(char*)IHM_REG_VAL_START_TIMEOUT, 
							&dwTemp)) != ERROR_SUCCESS)
		{
			TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_START_TIMEOUT));
			m_dwStartTimeout = 60000;
		}
		else
			m_dwStartTimeout = dwTemp;



// 		DWORD	m_dwStopTimeout;



// 		bool	m_bShowSplash;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_SHOW_SPLASH, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_SHOW_SPLASH));
		m_bShowSplash = true;
	}
	else
		m_bShowSplash = (dwTemp==0)?false:true;



// 		QString m_sUITemplateRoot;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_UI_TEMPLATE_ROOT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_UI_TEMPLATE_ROOT));
		return false;
	}

	m_sUITemplateRoot = MHelpFuncs::cleanAbsolutePath(szTemp);



// 		QString m_sUICssRoot;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_UI_CSS_ROOT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_UI_CSS_ROOT));
		return false;
	}

	m_sUICssRoot = MHelpFuncs::cleanAbsolutePath(szTemp);

// 		QString m_sUIInputExDefRoot;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_UI_INPUT_EX_DEF_ROOT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_UI_INPUT_EX_DEF_ROOT));
		return false;
	}

	m_sUIInputExDefRoot = MHelpFuncs::cleanAbsolutePath(szTemp);




	//m_sCommFilesRoot
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_COMM_FILES_ROOT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_COMM_FILES_ROOT));
		return false;
	}

	m_sCommFilesRoot = MHelpFuncs::cleanAbsolutePath(szTemp);


// 		QString m_sSplashTemplate;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_SPLASH_TEMPLATE, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_SPLASH_TEMPLATE));
		return false;
	}

	m_sSplashTemplate = QString(szTemp);

// 		QString m_sDefaultInputTemplate;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_INPUT_DLG_TEMPLATE, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_INPUT_DLG_TEMPLATE));
		return false;
	}

	m_sDefaultInputTemplate = QString(szTemp);

// 		QString m_sAboutDlgTemplate;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_ABOUT_DLG_TEMPLATE, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_ABOUT_DLG_TEMPLATE));
		return false;
	}

	m_sAboutDlgTemplate = QString(szTemp);

	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_ABOUT_DLG_DEFAULT_CFG,
		szTemp, 
		&dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_ABOUT_DLG_DEFAULT_CFG));
	}
	else
		m_sAboutDlgDefaultCfg = QString(szTemp);




	//QString m_sSystemEncoding
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_SYSTEM_ENCODING, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_SYSTEM_ENCODING));
		m_sSystemEncoding = "";
	}
	else
		m_sSystemEncoding = QString(szTemp);

	// 		DWORD	m_dwNumLockPeriod;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_NUMLOCK_PERIOD,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_NUMLOCK_PERIOD));
		m_dwNumLockPeriod = 1000;
	}
	else
		m_dwNumLockPeriod = dwTemp;
	
	// 		DWORD	m_dwNumLockSynchroKey;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_NUMLOCK_SYNCHKEY,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_NUMLOCK_SYNCHKEY));
		m_dwNumLockSynchroKey = 0;
	}
	else
		m_dwNumLockSynchroKey = dwTemp;
	
//-----------------------------------------

	// 		int	m_iDefaultResultPrecision;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_DEFAULT_RES_PRECISION,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_DEFAULT_RES_PRECISION));
		m_iDefaultResultPrecision = 2;
	}
	else
		m_iDefaultResultPrecision = dwTemp;


	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_DEFAULT_DECIMAL_SEPARATOR, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_DEFAULT_DECIMAL_SEPARATOR));
		m_cDefaultDecimalSeparator = ',';
	}
	else
		m_cDefaultDecimalSeparator = QChar(szTemp[0]);

	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_INPUTDLGS_ERROR_FILEDS_STYLE, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_INPUTDLGS_ERROR_FILEDS_STYLE));
		m_sInputDlgsErrorFieldsStyle = "background-color:rgb(255,224,187);";
	}
	else
		m_sInputDlgsErrorFieldsStyle = szTemp;


	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_INPUTDLGS_VALID_ENABLED, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]. Setting default to false!").arg(pcRegKey).arg( IHM_REG_VAL_INPUTDLGS_VALID_ENABLED));
		m_bInputDlgsValidAlwaysEnabled = false;
	}
	else
		m_bInputDlgsValidAlwaysEnabled = (dwTemp==0)?false:true;


	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_SHOW_CANCEL_ON_CONNECTING_DLG, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]: Setting default to false!").arg(pcRegKey).arg( IHM_REG_VAL_SHOW_CANCEL_ON_CONNECTING_DLG));
		m_bShowCancelOnConnectingDlg = false;
	}
	else
		m_bShowCancelOnConnectingDlg = (dwTemp==0)?false:true;


//-----------------------------------------


	// 		int	m_iAuthReqTimeout;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_AUTHORISATION_REQ_TIMEOUT,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]. Default 3000 is used!")
											.arg(pcRegKey).arg(IHM_REG_VAL_AUTHORISATION_REQ_TIMEOUT));
		m_iAuthReqTimeout = 0; //disable by default
	}
	else
	{
		m_iAuthReqTimeout = dwTemp;

		if(m_iAuthReqTimeout<0) //if the number is too big disable the timer
				m_iAuthReqTimeout = 0;
	}


//-----------------------------------------
// thread priorities
	//	QThread::Priority m_eAniThreadPriority; 
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_ANI_THREAD_PRIORITY,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]. Default QThread::NormalPriority (3) is used!")
											.arg(pcRegKey).arg(IHM_REG_VAL_ANI_THREAD_PRIORITY));
		m_eAniThreadPriority = QThread::NormalPriority; 
	}
	else
	{
		if(dwTemp<0||dwTemp>7) 
				m_eAniThreadPriority = QThread::NormalPriority;
		else
			m_eAniThreadPriority = (QThread::Priority)dwTemp;
	}
	
	
	//	QThread::Priority m_eWebThreadPriority; 
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_WEB_THREAD_PRIORITY,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]. Default QThread::NormalPriority (3) is used!")
											.arg(pcRegKey).arg(IHM_REG_VAL_WEB_THREAD_PRIORITY));
		m_eWebThreadPriority = QThread::NormalPriority; 
	}
	else
	{
		if(dwTemp<0||dwTemp>7) 
			m_eWebThreadPriority = QThread::NormalPriority;
		else
			m_eWebThreadPriority = (QThread::Priority)dwTemp;
	}

//-----------------------------------------
//  Load the WEB setting
//	bool	m_bWebInterfaceEnabled;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_WEB_ENABLED, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_WEB_ENABLED));
		m_bWebInterfaceEnabled = false;
	}
	else
		m_bWebInterfaceEnabled = (dwTemp==0)?false:true;


	
//-----------------------------------------
	//QString m_sLaneTypesCfgFile
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_LANE_TYPES_CFG_FILE, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_LANE_TYPES_CFG_FILE));
		return false;
	}
	else
		m_sLaneTypesCfgFile = MHelpFuncs::cleanAbsolutePath(szTemp);


	//Load all available lane types...
	if(!loadAvailableLaneTypesDsk(m_sLaneTypesCfgFile))
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadAvailableLaneTypes: Error loading \\LaneTypes in %1").arg(m_sModuleConfigKey));
		return false;
	}



//-----------------------------------------
//Load colours
// 		QString m_sDynamicColorsFile;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_DYNAMIC_COLOR_FILE, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_DYNAMIC_COLOR_FILE));
		return false;
	}

	m_sDynamicColorsFile = MHelpFuncs::cleanAbsolutePath(szTemp);


	if(!m_oConfigColors.loadConfigFromFile(m_sDynamicColorsFile))
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromFile: Error loading dynamic colors from %1!").arg(m_sDynamicColorsFile));
		return false;
	}


//-----------------------------------------
	// Language par defaut : string
	// 		QString m_sLanguage;
	dwValeurLen = sizeof (szTemp) ;
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_DEFAULT_LANGUAGE, 
                        szTemp, 
                        &dwValeurLen )) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_DEFAULT_LANGUAGE));
		return false;
	}
	m_sDefaultLanguage = szTemp;

	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_LANGUAGES_CFG_FILE, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_LANGUAGES_CFG_FILE));
		return false;
	}
	else
		m_sLanguagesCfgFilePath = MHelpFuncs::cleanAbsolutePath(szTemp);

	
	if(!m_oLanguages.initFromFile(m_sLanguagesCfgFilePath, m_sDefaultLanguage, false))
	{
        TRACE_W("MIhmConfigGeneral::loadConfigFromRegistry: ERREUR m_oLanguages.initFromFile returned false");
        return false;

	}	

//-----------------------------------------
//load image map table from XML file	
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_IMAGES_CFG_DSK, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]")
			.arg(pcRegKey).arg( IHM_REG_VAL_IMAGES_CFG_DSK));
		return false;
	}
	else
		m_sImagesCfgFilePath = MHelpFuncs::cleanAbsolutePath(szTemp);
	
	if(!m_oConfImages.initFromFile(m_sImagesCfgFilePath))
	{
        TRACE_W(QString("MIhmConfigGeneral::loadConfigFromRegistry:LoadDskConfigImages %1 returned false!")
			.arg(m_sImagesCfgFilePath));
        return false;
	}	

//-----------------------------------------
	//init action
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_ACTIONS_CFG, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]")
			.arg(pcRegKey).arg( IHM_REG_VAL_ACTIONS_CFG));
		return false;
	}
	else
		m_sActionsCfgFilePath = MHelpFuncs::cleanAbsolutePath(szTemp);
	
	if(!m_oConfigActions.loadConfigFromFile(m_sActionsCfgFilePath))
	{
        TRACE_W(QString("MIhmConfigGeneral::loadConfigFromRegistry: Error loading actions from file %1")
			.arg(m_sActionsCfgFilePath));
		return false;
	}
//-----------------------------------------
	
	//init cursors
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_CURSOR_CFG_DSK, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString("MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]")
			.arg(pcRegKey).arg( IHM_REG_VAL_CURSOR_CFG_DSK));
		return false;
	}
	else
		m_sCursorsCfgFilePath = MHelpFuncs::cleanAbsolutePath(szTemp);

	if(!m_oConfigCursors.loadConfigFromFile(m_sCursorsCfgFilePath))
	{
        TRACE_W(QString("MIhmConfigGeneral::loadConfigFromRegistry:m_oConfigCursors.loadConfigFromFile %1 returned false!")
													.arg(m_sCursorsCfgFilePath));
        return false;
	}	


//-----------------------------------------

	//init dynamic
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_DYNAMIC_CFG, 
		szTemp, 
		&dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]")
			.arg(pcRegKey).arg( IHM_REG_VAL_DYNAMIC_CFG));
		return false;
	}
	else
		m_sDynamicCfgFilePath = MHelpFuncs::cleanAbsolutePath(szTemp);
	
	if(!reloadConfigDynamic())
	{
        return false;
	}	

//-----------------------------------------


// 		QString m_sDefaultVideoDir;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_DEFAULT_VIDEO_DIR,
		szTemp, 
		&dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! Using CommFilesRoot.")
			.arg(pcRegKey).arg(IHM_REG_VAL_DEFAULT_VIDEO_DIR));
		m_defaultVideoFullPath = m_sCommFilesRoot;
	}

	m_defaultVideoFullPath = getCommFileFullPath(szTemp);


//-----------------------------------------

	// 		DWORD	m_dwMaxInputMsgPerSec;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_MAX_INPUT_MSGS_PER_SEC,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! Using default 3!")
			.arg(pcRegKey).arg( IHM_REG_VAL_MAX_INPUT_MSGS_PER_SEC));
		m_dwMaxInputMsgPerSec = 3;
	}
	else
		m_dwMaxInputMsgPerSec = dwTemp;


//---------------------------------------------

	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_MAX_TRACE_SIZE_MB,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! Using default 1!")
			.arg(pcRegKey).arg( IHM_REG_VAL_MAX_TRACE_SIZE_MB));
	}
	else
		MTracer::getTracer()->setMaxTraceSize(dwTemp);


//----------------------------------------------------------
	
// 		QString m_sTestKeyboardStrDetMaskRegex;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_TEST_KEYBOARD_MASK_REGEX,
		szTemp, 
		&dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! No string detection will be masked in TestKeyboard form!")
					.arg(pcRegKey).arg(IHM_REG_VAL_TEST_KEYBOARD_MASK_REGEX));
	}
	else
		m_sTestKeyboardStrDetMaskRegex = szTemp;

	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_TEST_KEYBOARD_MASK_ENABLED,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! Using default 1!")
			.arg(pcRegKey).arg( IHM_REG_VAL_TEST_KEYBOARD_MASK_ENABLED));
		
		m_dwTestKeyboardMaskEnabled = 1;
	}
	else
		m_dwTestKeyboardMaskEnabled = dwTemp;

	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_TEST_KEYBOARD_MASK_CHAR,
		szTemp, 
		&dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! !")
					.arg(pcRegKey).arg(IHM_REG_VAL_TEST_KEYBOARD_MASK_CHAR));
		
		m_chTestKeyboardMaskChar = '=';
	}
	else 
	{
		if(szTemp[0]!='\0')
			m_chTestKeyboardMaskChar = szTemp[0];
		else
			m_chTestKeyboardMaskChar = '=';
	}


	//bool		m_bFilterStringDetectedKeys;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_FILTER_STRING_DETECTED_KEYS,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! Using default:true!")
			.arg(pcRegKey).arg( IHM_REG_VAL_FILTER_STRING_DETECTED_KEYS));
		
		m_bFilterStringDetectedKeys = true;
	}
	else
		m_bFilterStringDetectedKeys = (dwTemp==0)?false:true;

//-------------------------------------------------------------------------------------
// Virtual keyboard parameters


	// 		bool	m_bVirtualKeyboardEnabled;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_VIRTUAL_KEYBOARD_ENABLED,
		&dwTemp)) != ERROR_SUCCESS)
	{
		TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_VIRTUAL_KEYBOARD_ENABLED));
		m_bVirtualKeyboardEnabled = false;
	}
	else
		m_bVirtualKeyboardEnabled = (dwTemp==0)?false:true;


	return true;
}

bool MIhmConfigGeneral::reloadConfigDynamic()
{

	if(!m_oConfigColors.loadConfigFromFile(m_sDynamicColorsFile))
	{
        TRACE_W(QString( "MIhmConfigGeneral::reloadConfigDynamic: Error loading dynamic colors from %1!").arg(m_sDynamicColorsFile));
		return false;
	}
	
	if(!m_oConfigDynamic.loadConfigFromFile(m_sDynamicCfgFilePath))
	{
        TRACE_W(QString( "MIhmConfigGeneral::reloadConfigDynamic:m_oConfigDynamic.loadConfigFromFile %1 returned false!")
									.arg(m_sDynamicCfgFilePath));
        return false;
	}

	return true;
}



bool MIhmConfigGeneral::addVideoZoomParams(QString sVideoZoomStr)
{
 	QStringList lstZoom = sVideoZoomStr.split(",");

	QRect rctVideoParam;

	if(lstZoom.size() == 4)
	{
		rctVideoParam.setX(lstZoom.at(0).toInt());
		rctVideoParam.setY(lstZoom.at(1).toInt());
		rctVideoParam.setWidth(lstZoom.at(2).toInt());
		rctVideoParam.setHeight(lstZoom.at(3).toInt());

		m_lstVideoZoomParams.append(rctVideoParam);
		return true;
	}
	else
	{
		return false;
	}

}



bool MIhmConfigGeneral::getVideoZoomParams(int iIndex, QRect &rctZoomParams)
{
	if(iIndex<m_lstVideoZoomParams.size()&& iIndex>=0)
	{
		rctZoomParams = m_lstVideoZoomParams.at(iIndex);
		return true;
	}	
	else
		return false;
}





bool MIhmConfigGeneral::loadAvailableLaneTypesDsk(QString sLaneTypesCfgFilePath)
{
    TRACE_D("MIhmConfigGeneral::loadAvailableLaneTypesDsk..." );

	bool bRet = false;
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument xmlData;

	QFile file(sLaneTypesCfgFilePath);
	
	if(file.open(QFile::ReadOnly))
	{

		bRet = xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn);

		if(bRet)
		{
			QDomElement root = xmlData.documentElement();
			if(root.tagName() != XML_ELEMENT_IHM_CONFIG_ROOT)
			{
				TRACE_W("MIhmConfigGeneral::loadAvailableLaneTypesDsk: Invalid root element!");
				bRet = false;
			}
			else
			{
				QDomNodeList lstLanesItems = root.elementsByTagName(XML_ELEMENT_IHM_CONFIG_LANE_TYPES);
				
				if(lstLanesItems.count()>0)
				{
					QDomNode laneTypes = lstLanesItems.at(0);
					//root element for all lane types
					QDomElement elLaneTypes = laneTypes.toElement();

					QDomNodeList lstLaneItems = elLaneTypes.elementsByTagName(XML_ELEMENT_IHM_CONFIG_LANE_TYPE);
					
					QDomNode currLane; 
					
					for(int i=0;i<lstLaneItems.count();i++)
					{
						currLane = lstLaneItems.at(i); 

						QString sLaneTypeID = MHelpFuncs::getAttributeText(&currLane, XML_ATTRIBUTE_ID);
						
						
						//load DESKTOP settings
						MIhmLaneTypeSettings * pNewLaneType = new MIhmLaneTypeSettings(MIhmLaneTypeSettings::enuDESKTOP, sLaneTypeID);
						
						pNewLaneType->loadXMLSettings(currLane);

						m_lstLaneTypeSettings.append(pNewLaneType);

					}

					
				}
					

			}
		}
		else
		{
			QString sMsg = QString("MIhmConfigGeneral::OpenXML:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
			TRACE_W(sMsg);
		}
	}
	else
	{
		TRACE_W(QString( "MIhmConfigGeneral::OpenXML: Unable to open file %1").arg(sLaneTypesCfgFilePath) );
	}

	return bRet;
	

}

// bool MIhmConfigGeneral::loadConfigFromFile(QString sConfigFilenameAndPath)
// {
// 	QString errorStr;
// 	int errorLine;
// 	int errorColumn;
// 	QDomDocument xmlData;
// 
// 	QFile file(sConfigFilenameAndPath);
// 	
// 	if(file.open(QFile::ReadOnly))
// 	{
// 		
// 		if(xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn))
// 		{
// 			QDomElement root = xmlData.documentElement();
// 			
// 			if(QString::compare(root.tagName(), "CONFIG", Qt::CaseInsensitive)!=0)
// 			{
// 				TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromXml: Invalid root element!");
// 				return false;
// 			}
// 		}
// 		else
// 		{
// 			QString sMsg = QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
// 			TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromXml: %s",sMsg.toLatin1().data() );
// 			return false;
// 		}
// 	}
// 	else
// 	{
// 		TRACE_W(QString( "MMenuFileView::OpenXML: Unable to open file %s",sConfigFilenameAndPath.toLatin1().data() );
// 		return false;
// 	}
// 	
// 	QDomElement root = xmlData.documentElement();
// 	QDomNodeList lstItems = root.childNodes();
// 	QDomNode currItem;
// 		
// 	for(int i=0;i<lstItems.count();i++)
// 	{
// 		currItem = lstItems.at(i); 
// 
// 		// here we go through all parameters that we want to load
// 
// 		/*	
// 		if(QString::compare(currItem.nodeName(), "PARAM",Qt::CaseInsensitive)==0)
// 		{
// 			// check param attribute name and get value 
// 			QString sParamID = MHelpFuncs::getAttributeText(&currItem, "ID");
// 			QString sParamValue = MHelpFuncs::getAttributeText(&currItem, "VALUE");
// 	
// 			if(QString::compare(sParamID, "MaxWebClients",Qt::CaseInsensitive)==0)
// 			{
// 				bool ok;
// 				
// 				m_dwMaxWebClients = sParamValue.toInt(&ok, 10);
// 				
// 				if(!ok)
// 				{
// 					TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromXml: Error param %s[%s]", "MaxWebClients",sParamValue.toLatin1().data() );
// 					return false;
// 				}
// 			}
// 
// 		}
// 		*/
// 	}
// 
// 
// 	return true;
// }

MIhmLaneTypeSettings * MIhmConfigGeneral::getLaneTypeSetting(MIhmLaneTypeSettings::enumSettingsType eType,
																 QString sLaneType)
{
	MIhmLaneTypeSettings * pRetVal = NULL;

	for (int i=0;i<m_lstLaneTypeSettings.size();++i)
	{
		if(m_lstLaneTypeSettings.at(i)->getLaneType()==sLaneType &&
			m_lstLaneTypeSettings.at(i)->getSettingType() == eType)
		{
			pRetVal = m_lstLaneTypeSettings.at(i);
			break;
		}
	}

	return pRetVal;
}




bool MIhmConfigGeneral::parseDefaultFontConfig(QString sFont)
{
	bool bRetValue = false;
	MPoliceValue oPolVal;
	oPolVal.updateWithString(sFont);

	if(oPolVal.isFontValid())
	{
		bRetValue = true;
	}

	return bRetValue;
} 

QString MIhmConfigGeneral::getCleanPath(QString sPath, QString sSubPath)
{
	QString sRet;

	sRet = MHelpFuncs::cleanAbsolutePath(sPath);

	if (!sSubPath.isEmpty())
		sRet = QDir::cleanPath(sRet + QDir::separator() + sSubPath);

	return sRet;
}


QString MIhmConfigGeneral::getUIFullPath(QString sUI)
{
	return getCleanPath(m_sUITemplateRoot, sUI);
}


QString MIhmConfigGeneral::getCSSFullPath(QString sCSSFile)
{
	return getCleanPath(m_sUICssRoot, sCSSFile);
}


QString MIhmConfigGeneral::getUIInputExDefPath(QString sFileName)
{
	return getCleanPath(m_sUIInputExDefRoot, sFileName);
}


QString MIhmConfigGeneral::getCommFileFullPath(QString sDataFile)
{
	return getCleanPath(m_sCommFilesRoot, sDataFile);
}





bool MIhmConfigGeneral::getPoliceFontValue(MPoliceValue * pPoliceValue, QFont *pFnt)
{
	bool bRetVal = false;

	if(pPoliceValue->isFontValid())
	{
		if(m_pGenCfg!=NULL)
		{
			*pFnt = m_pGenCfg->m_sDefaultFont; //...geting the  DefaultFont
			bRetVal = true;
		}
		else
			return false;

		QString sFamily = pPoliceValue->getFontFamily();
		if(sFamily!="")
				pFnt->setFamily(sFamily);

		int iPointSize = pPoliceValue->getPointSize();
		if(iPointSize>0)
			pFnt->setPointSize(iPointSize);

		int iFontWeight = pPoliceValue->getFontWeight();
		if(iFontWeight!=UNDEFINED_POLICE_VALUE)
		{	pFnt->setWeight((QFont::Weight)iFontWeight);
			
			if(iFontWeight>50)
				pFnt->setBold(true);
			else
				pFnt->setBold(false);
		}
		
		int iUnderline = pPoliceValue->getUnderline();
		if(iUnderline!=UNDEFINED_POLICE_VALUE)
			pFnt->setUnderline(iUnderline>0?true:false);

		int iItalic = pPoliceValue->getItalic();
		if(iItalic!=UNDEFINED_POLICE_VALUE)
			pFnt->setItalic(iItalic>0?true:false);

		int iStrikeOut = pPoliceValue->getStrikeOut();
		if(iStrikeOut!=UNDEFINED_POLICE_VALUE)
			pFnt->setStrikeOut(iStrikeOut>0?true:false);

	}


	return bRetVal;
}



QString MIhmConfigGeneral::getPoliceFontStyle(MPoliceValue * pPoliceValue)
{
	QString sFontStyle;
	MPoliceValue oDefaultPolVal;

	if(pPoliceValue->isFontValid())
	{
		if(m_pGenCfg!=NULL)
		{
			QString sDefaultFont = m_pGenCfg->m_sDefaultFont; //...geting the  DefaultFont
			oDefaultPolVal.updateWithString(sDefaultFont);
		}
		else
			return "";


		QString sFamily = pPoliceValue->getFontFamily();
		if(sFamily=="")
			sFamily = oDefaultPolVal.getFontFamily();
		

		int iPointSize = pPoliceValue->getPointSize();
		
		if(iPointSize<=0)
			iPointSize = oDefaultPolVal.getPointSize();


		int iFontWeight = pPoliceValue->getFontWeight();
		if(iFontWeight==UNDEFINED_POLICE_VALUE)
		{	
			iFontWeight = oDefaultPolVal.getFontWeight();
		}
		
		int iItalic = pPoliceValue->getItalic();
		if(iItalic == UNDEFINED_POLICE_VALUE)
			iItalic = oDefaultPolVal.getItalic();

		int iUnderline = pPoliceValue->getUnderline();
		if(iUnderline==UNDEFINED_POLICE_VALUE)
			iUnderline = oDefaultPolVal.getUnderline();

		int iStrikeOut = pPoliceValue->getStrikeOut();
		if(iStrikeOut==UNDEFINED_POLICE_VALUE)
			iStrikeOut = oDefaultPolVal.getStrikeOut();


		sFontStyle = QString("font:%1 %2 %3pt \"%4\";")
			.arg((iFontWeight>50)?"bold":"")
			.arg((iItalic>0)?"italic":"")
			.arg(iPointSize)
			.arg(sFamily);


		if(iUnderline>0 || iStrikeOut>0)
		{
			sFontStyle += QString("text-decoration: %1 %2;")
				.arg(iUnderline>0?"underline":"")
				.arg(iStrikeOut>0?"line-through":"");
		}
	}


	return sFontStyle;
}



QCursor MIhmConfigGeneral::getCursor(QString sCursorId)
{
	QCursor cursor = m_oConfigCursors.getCursor(&m_oConfImages,sCursorId);

	return cursor;
}

QString MIhmConfigGeneral::createDefaultVideoFullPath(QString sImageName) 
{ 
	QDir oDir; 

	if(!oDir.mkpath(m_defaultVideoFullPath))
	{
		TRACE_W(QString( "MIhmConfigGeneral::createDefaultVideoFullPath: - Unable to create folder [%s]!").arg(m_defaultVideoFullPath));
		//return false;
	}

	return getCleanPath(m_defaultVideoFullPath, sImageName); 
}

