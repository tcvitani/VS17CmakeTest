

#include "MIhmAniInitData.h"
#include "MTracer.h"
#include <QDir>
#include "MHelpFuncs.h"

extern "C" {
	#include <ihm.H>
	#include "ihm_loc.h"
	#include <reg.h>
};


MIhmAniInitData::MIhmAniInitData()
{
	
}


MIhmAniInitData::~MIhmAniInitData()
{

}

QString MIhmAniInitData::getMboxName()
{
	return m_sMboxName;
}


bool MIhmAniInitData::initialize(QString szMboxName, QString sConfigKey)
{
    DWORD dwValeurLen;
    DWORD dwRes;
	char pcRegKey[IHM_LG_LIGNE_MAX] = {0};
    char szWork[IHM_LG_LIGNE_MAX];
    char szTemp[MAX_PATH];


	m_sMboxName = szMboxName;
	m_sConfigKey = sConfigKey;

	strcpy_s(pcRegKey, sizeof(pcRegKey)-1, sConfigKey.toLatin1().data());

	// Pool utilise : string
	// 	QString m_sPool;
	dwValeurLen = sizeof (szWork) ;
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)MOD_REG_KEYv_POOL, 
                        szWork, 
                        (unsigned long*)&dwValeurLen ) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmAniInitData::initialize: Erreur cle %1[%2]").arg(pcRegKey).arg(MOD_REG_KEYv_POOL));
		return false;
	}

	m_iPoolId = NOYAU_GetPoolId (szWork) ;
	
	DWORD dwTemp;
	//		DWORD	m_dwVideoZoomNo;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_VIDEO_ZOOM_NO,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmAniInitData::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg(IHM_REG_VAL_VIDEO_ZOOM_NO));
		return false;
	}
	m_dwVideoZoomNo = dwTemp;


	//m_sCommFilesRoot
	dwValeurLen = sizeof (szWork);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_COMM_FILES_ROOT, 
                        szWork, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmAniInitData::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_COMM_FILES_ROOT));
		return false;
	}

	m_sCommFilesRoot = MHelpFuncs::cleanAbsolutePath(szWork);


	//		DWORD	m_dwMaxTableRowsInMemory;
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		(char*)IHM_REG_VAL_VIDEO_MAX_TABLE_ROWS_IN_MEMORY,
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmAniInitData::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg(IHM_REG_VAL_VIDEO_MAX_TABLE_ROWS_IN_MEMORY));
		m_dwMaxTableRowsInMemory = 20;
	}
	else
		m_dwMaxTableRowsInMemory = dwTemp;


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


	if(m_bVirtualKeyboardEnabled)
	{
		dwValeurLen = sizeof (szTemp);
		if ((dwRes = REG_Lire_Chaine (
			CSR_REG_KEYi_ROOT, 
			pcRegKey,
			(char*)IHM_REG_VAL_VIRTUAL_KEYBOARD_MBOX_NAME,
			szTemp, 
			&dwValeurLen) ) != ERROR_SUCCESS)
		{
			TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2] - Virtual keyboard mbox is not defined! It should be defined if %3 parameter is enabled!")
				.arg(pcRegKey).arg(IHM_REG_VAL_VIRTUAL_KEYBOARD_MBOX_NAME).arg(IHM_REG_VAL_VIRTUAL_KEYBOARD_ENABLED));
			return false;
		}
		else
			m_sVirtKeybMboxName = szTemp;

		QString sVirtKybDef;

		dwValeurLen = sizeof (szTemp);
		if ((dwRes = REG_Lire_Chaine (
			CSR_REG_KEYi_ROOT, 
			pcRegKey,
			(char*)IHM_REG_VAL_VIRTUAL_KEYBOARD_DEFAULT_CFG,
			szTemp, 
			&dwValeurLen) ) != ERROR_SUCCESS)
		{
			TRACE_W(QString( "MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_VIRTUAL_KEYBOARD_DEFAULT_CFG));
			return false;
		}
		else
		{	sVirtKybDef = szTemp;

			if(!sVirtKybDef.isEmpty())
			{
				QString sParam1 = MHelpFuncs::getStringItem(sVirtKybDef, 0, QString(IHM_SAISIE_EX_SEPARATEUR_PARAM), true);
				QString sParam2 = MHelpFuncs::getStringItem(sVirtKybDef, 1, QString(IHM_SAISIE_EX_SEPARATEUR_PARAM), true);
				QString sParam3= MHelpFuncs::getStringItem(sVirtKybDef, 2, QString(IHM_SAISIE_EX_SEPARATEUR_PARAM), true);

				m_sVirtKeyDefaultCfg = sParam1;
				m_iVirtKeyDefaultPosX = sParam2.toInt();
				m_iVirtKeyDefaultPosY = sParam3.toInt();
			}
		}	

	}



//-------------------------------------------------------------------------------------

	return INST_INIT_OK;
}



QString MIhmAniInitData::getCommFileFullPath(QString sDataFile)
{
	return getCleanPath(m_sCommFilesRoot, sDataFile);
}


QString MIhmAniInitData::getCleanPath(QString sPath, QString sSubPath)
{
	QString sRet;

	sPath = sPath.trimmed();

	if(sPath.size()>0)
		if(sPath.right(1) == QString("/") || sPath.right(1) == QString("\\"))
			sPath = sPath.left(sPath.size()-1);

	if (sPath.at(0) == QChar('.') && sPath.at(1) != QChar('.'))
		sPath = QDir::cleanPath(QDir::currentPath() + QDir::separator() + QDir::cleanPath(sPath));
	else
		sPath = QDir::cleanPath(sPath);

	if (!sSubPath.isEmpty())
		sRet = QDir::cleanPath(sPath + QDir::separator() + sSubPath);
	else
		sRet = sPath;

	return sRet;
}