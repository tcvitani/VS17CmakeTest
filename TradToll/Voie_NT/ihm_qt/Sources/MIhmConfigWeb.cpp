

#include "MIhmConfigWeb.h"
#include "MHelpFuncs.h"
#include "MTracer.h"
#include "MParamList.h"

#include <QFont>
#include <QStringList>
#include <QDir>
#include <QColor>

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <ihm.H>
	#include "ihm_loc.h"
	#include <reg.h>
	#include <csr_srv.h>

};

MIhmConfigWeb * MIhmConfigWeb::m_pWebCfg = NULL;

MIhmConfigWeb::MIhmConfigWeb()
{
	
}


MIhmConfigWeb::~MIhmConfigWeb()
{
	while(!m_lstLaneTypeSettings.isEmpty())
		delete m_lstLaneTypeSettings.takeLast();
	
}

QString MIhmConfigWeb::getCommFileFullPath(QString sDataFile)
{
	QString sRet;

	if(sDataFile != "")
		sRet = QDir::cleanPath(m_sCommFilesRoot + QDir::separator() + sDataFile); 

	return sRet;
	
}

bool MIhmConfigWeb::loadConfigFromRegistry(QString sRegKey)
{
    DWORD dwValeurLen;
    DWORD dwRes;
    DWORD dwTemp;
    char szTemp[IHM_LG_LIGNE_MAX] = {0};
	char pcRegKey[IHM_LG_LIGNE_MAX] = {0};

    TRACE_WEB_D("MIhmConfigWeb::loadConfigFromRegistry" );

	//make this webConfig object public by assigning to the public static variable
	m_pWebCfg = this; 

	m_sModuleConfigKey = sRegKey;
	strcpy_s(pcRegKey, sizeof(pcRegKey), sRegKey.toLatin1().data());


	//QString m_sHtmlEncMapFile
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_HTML_ENC_MAP_FILE, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_HTML_ENC_MAP_FILE));
		return false;
	}
	else
	{
		m_sHtmlEncMapFile = QString(szTemp);
		m_oHtmlCodec.init(m_sHtmlEncMapFile);
	}



//---Load translation languages from XML file
	// Default language
	dwValeurLen = sizeof (szTemp) ;
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_DEFAULT_LANGUAGE, 
                        szTemp, 
                        &dwValeurLen )) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_DEFAULT_LANGUAGE));
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
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_LANGUAGES_CFG_FILE));
		return false;
	}
	else
		m_sLanguagesCfgFilePath = QString(szTemp);

	if(!m_oLanguages.initFromFile(m_sLanguagesCfgFilePath, m_sDefaultLanguage, true))
	{
        TRACE_WEB_W(QString( "MIhmWebInterface::initialize: m_oLanguages.initFromFile:[%1], Langage:[%2] returned false!").
									arg(m_sLanguagesCfgFilePath).
									arg(m_sDefaultLanguage));
        return false;
	}	
//---END Load translation languages 



//------------------------------------------------
// --- load web config images
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_IMAGES_CFG_WEB, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_IMAGES_CFG_WEB));
		return false;
	}
	else
		m_sImagesCfgFilePath = QString(szTemp);
	

	if(m_oWebImages.initFromFile(m_sImagesCfgFilePath) == false)
	{
        TRACE_WEB_W(QString( "MIhmWebInterface::initialize: LoadWebConfigImages [%1] return false!").
									arg(m_sImagesCfgFilePath));
        return false;
	}	
	

//------------------------------------------------
//---Load lane types setting from XML file
	//QString m_sLaneTypesCfgFile
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_LANE_TYPES_CFG_FILE, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_LANE_TYPES_CFG_FILE));
		return false;
	}
	else
		m_sLaneTypesCfgFile = QString(szTemp);


	//Load all available lane types...
	if(!loadAvailableLaneTypesWeb(m_sLaneTypesCfgFile))
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error loadAvailableLaneTypesWeb loading %1 returned false!").
									arg(m_sLaneTypesCfgFile));
		return false;
	}

//---END ---- Load lane types setting from XML file


//-----------------------------------------
// 		QString m_sDynamicColorsFile;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_DYNAMIC_COLOR_FILE, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_DYNAMIC_COLOR_FILE));
		return false;
	}

	m_sDynamicColorsFile = QString(szTemp);


	if(!m_oConfigColors.loadConfigFromFile(m_sDynamicColorsFile))
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromFile: Error loading dynamic colors from %1!").
									arg(m_sDynamicColorsFile));
		return false;
	}

//-----------------------------------------



	//Load other WEB settings...
	
	//m_sInitWebPage
	dwValeurLen = sizeof (szTemp) ;
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_INIT_MAIN_PAGE, 
                        szTemp, 
                        &dwValeurLen )) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_INIT_MAIN_PAGE));
		return false;
	}
	m_sInitWebPage = szTemp;


	//m_sLoginFailPage
	dwValeurLen = sizeof (szTemp) ;
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_LOGIN_FAIL_PAGE, 
                        szTemp, 
                        &dwValeurLen )) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_LOGIN_FAIL_PAGE));
		return false;
	}
	m_sLoginFailPage = szTemp;


	//m_sLogoutPage
	dwValeurLen = sizeof (szTemp) ;
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_LOGOUT_PAGE, 
                        szTemp, 
                        &dwValeurLen )) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_LOGOUT_PAGE));
		return false;
	}
	m_sLogoutPage = szTemp;




//	int		m_iWebServerPort;  
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_WEB_PORT, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_WEB_PORT));
		m_iWebServerPort = 8080;
	}
	else
		m_iWebServerPort = dwTemp;

//	int		m_iWebSessionTimeout;  
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_WEB_SESSION_TIMEOUT, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_WEB_SESSION_TIMEOUT));
		m_iWebSessionTimeout = 5;
	}
	else
		m_iWebSessionTimeout = dwTemp;

//	int		m_iMiniWebSessionTimeout;  
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_MINI_WEB_SESSION_TIMEOUT, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_MINI_WEB_SESSION_TIMEOUT));
		m_iMiniWebSessionTimeout = 5;
	}
	else
		m_iMiniWebSessionTimeout = dwTemp;



	//m_sCommFilesRoot
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_COMM_FILES_ROOT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_COMM_FILES_ROOT));
		return false;
	}
	m_sCommFilesRoot = QString(szTemp);

	
	//-------------------------------------------------------------------------------------------
	//init action
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_ACTIONS_CFG, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_ACTIONS_CFG));
		return false;
	}
	else
		m_sActionsCfgFilePath = QString(szTemp);
	
	if(!m_oConfigActions.loadConfigFromFile(m_sActionsCfgFilePath))
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error loading actions from file %1").
									arg(m_sActionsCfgFilePath));
		return false;
	}	
	//-------------------------------------------------------------------



	// 	m_sDefaultDateFormat;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_DEFAULT_DATE_FORMAT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_DEFAULT_DATE_FORMAT));
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
        TRACE_WEB_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_DEFAULT_TIME_FORMAT));
		return false;
	}

	m_sDefaultTimeFormat = szTemp;

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
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_DYNAMIC_CFG));
		return false;
	}
	else
		m_sDynamicCfgFilePath = QString(szTemp);
	
	if(!reloadDynamicConfig())
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry:m_oConfigDynamic.loadConfigFromFile %1 returned false!").
									arg(m_sDynamicCfgFilePath));
        return false;
	}	

//-----------------------------------------


	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_INPUTDLGS_VALID_ENABLED, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_INPUTDLGS_VALID_ENABLED));
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
        TRACE_WEB_W(QString( "MIhmConfigWeb::loadConfigFromRegistry: Error key %1[%2]: Setting default to false!").arg(pcRegKey).arg( IHM_REG_VAL_SHOW_CANCEL_ON_CONNECTING_DLG));
		m_bShowCancelOnConnectingDlg = false;
	}
	else
		m_bShowCancelOnConnectingDlg = (dwTemp==0)?false:true;


	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_MAX_TRACE_SIZE_WEB_MB,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! Using default 1!").arg(pcRegKey).arg( IHM_REG_VAL_MAX_TRACE_SIZE_WEB_MB));
	}
	else
		MTracer::getWebTracer()->setMaxTraceSize(dwTemp);


	return true;
}

bool  MIhmConfigWeb::reloadDynamicConfig()
{
	if(!m_oConfigColors.loadConfigFromFile(m_sDynamicColorsFile))
	{
        TRACE_WEB_W(QString( "MIhmConfigWeb::reloadDynamicConfig: Error loading dynamic colors from %1!").
									arg(m_sDynamicColorsFile));
		return false;
	}

	return m_oConfigDynamic.loadConfigFromFile(m_sDynamicCfgFilePath);
}


bool MIhmConfigWeb::loadAvailableLaneTypesWeb(QString sLaneTypesCfgFilePath)
{
    TRACE_WEB_D("MIhmConfigWeb::loadAvailableLaneTypesWeb..." );

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
				TRACE_WEB_W("MIhmConfigWeb::loadAvailableLaneTypesWeb: Invalid root element!");
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
						
						
						//load WEB settings
						MIhmLaneTypeSettings * pNewLaneType = new MIhmLaneTypeSettings(MIhmLaneTypeSettings::enuWEB, sLaneTypeID);
						
						pNewLaneType->loadXMLSettings(currLane);

						m_lstLaneTypeSettings.append(pNewLaneType);

					}
				}
			}
		}
		else
		{
			QString sMsg = QString("loadAvailableLaneTypesWeb::OpenXML:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
			TRACE_WEB_W(sMsg);
		}
	}
	else
	{
		TRACE_WEB_W(QString( "loadAvailableLaneTypesWeb::OpenXML: Unable to open file %1").arg(sLaneTypesCfgFilePath) );
	}

	return bRet;
}



MIhmLaneTypeSettings * MIhmConfigWeb::getLaneTypeSetting(MIhmLaneTypeSettings::enumSettingsType eType,
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


bool MIhmConfigWeb::getGenericScriptParameters(QString sScriptVisibleObjectName,  MParamList *pLstParams)
{
	QString sScriptObjectKey = QString("%1\\GenericScripts\\%2").arg(m_sModuleConfigKey).arg(sScriptVisibleObjectName);

	return pLstParams->loadListFromRegistry(sScriptObjectKey);
}




bool MIhmConfigWeb::parsePoliceValueForWeb(QString sPoliceValue, QString& sStyle)
{
	bool bRetVal = true;
	QStringList lst;

	sStyle = "";

// 	QString sValue = QString("%1,%2,%3,%4,%5,%6,%7")
// 							.arg(m_sFontFamily)
// 							.arg(m_iPointSize)
// 							.arg(m_iUnderline)
// 							.arg(m_iItalic)
// 							.arg(m_iStrikeOut)
// 							.arg(m_sForeColor)
// 							.arg(m_sBckColor);

	lst = sPoliceValue.split(IHM_REG_FONT_SEPARATOR);

	//lst.at(0) //family

	//Point size
	if(lst.size()>1)
	{
		bool bOK;
		int iSize = lst.at(1).toInt(&bOK);
	
		if (bOK && iSize < 100)
		{
			// 	pFnt->setSize(iWeight);
		}
		//else
			//bRetVal = false;
	}

	//wight
	if(lst.size()>2)
	{
		bool bOK;
		int iWeight = (lst.at(2).toInt(&bOK))/10;

 		if(bOK && iWeight<100)
		{
// 			pFnt->setWeight(iWeight);
		}
		else
			bRetVal = false;
	}

	//underline
	if(lst.size()>3)
	{
// 		pFnt->setUnderline((lst.at(3).toInt()==1)?true:false);
	}
	
	//italic
	if(lst.size()>4)
	{
// 		pFnt->setItalic((lst.at(4).toInt()==1)?true:false);
	}

	//strike out
	if(lst.size()>5)
	{
// 		pFnt->setStrikeOut((lst.at(5).toInt()==1)?true:false);
	}

	//colour
	if(lst.size()>5)
	{
		bool bOK;
		int iColour = lst.at(6).toInt(&bOK,16);
		
		if(bOK)
		{
			sStyle += MIhmConfigColor::createColorStyle(MIhmConfigColor::makeHexColorString(iColour));
		}
		else
			bRetVal = false;
	}

	if(!bRetVal)
	{
		TRACE_WEB_W(QString( "MIhmConfigWeb::parsePoliceValueForWeb: Unable to parse police value: %1").arg(sPoliceValue));
		bRetVal = false;

	}

	return bRetVal;	
}




QString MIhmConfigWeb::htmlEncodeStr(QString str)
{

	return m_oHtmlCodec.htmlEncodeStr(str);
}

