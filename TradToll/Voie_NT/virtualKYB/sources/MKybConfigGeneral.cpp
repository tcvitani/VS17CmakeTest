

#include "MKybConfigGeneral.h"
#include "MTracer.h"
#include "MKybTypeSettings.h"
#include <QCoreApplication>

#include <QFont>
#include <QStringList>
#include <QDir>
#include <QColor>
#include <QSettings>


extern "C" {
	#include <virt_kyb.h>
	#include <reg.h>
	#include <noyau.h>
};

MKybConfigGeneral * MKybConfigGeneral::m_pGenCfg = NULL;



MKybConfigGeneral::MKybConfigGeneral()
{
	m_sModuleMboxName = "";
	m_sModuleConfigKey = "";
	m_eAniThreadPriority = QThread::NormalPriority; 
}


MKybConfigGeneral::~MKybConfigGeneral()
{
	
	while (!m_lstKybSettings.isEmpty())
		delete m_lstKybSettings.takeFirst();

}



bool MKybConfigGeneral::loadConfigFromRegistry(QString sMboxName)
{
    DWORD dwValeurLen;
    DWORD dwRes;
    DWORD dwTemp;
    char szTemp[MAX_PATH];
    char pcRegKey[MAX_PATH];

	//make this ConfigGeneral object public by assigning to the public static variable
	m_pGenCfg = this;
	m_sModuleMboxName = sMboxName;

    // module config key
    m_sModuleConfigKey = QString("%1%2").arg(VIRT_KYB_REG_KEY_BASE)
										.arg(m_sModuleMboxName);
	
	strcpy(pcRegKey, m_sModuleConfigKey.toAscii().data());

    TRACE_D("MKybConfigGeneral::loadConfigFromRegistry..." );
// 	QString m_sUITemplateRoot;
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        KYB_REG_VAL_UI_TEMPLATE_ROOT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MKybConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( KYB_REG_VAL_UI_TEMPLATE_ROOT));
		return false;
	}

	m_sUITemplateRoot = QString(szTemp);

	//m_sCommFilesRoot
	dwValeurLen = sizeof (szTemp);
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        KYB_REG_VAL_CFG_FILES_ROOT, 
                        szTemp, 
                        &dwValeurLen) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MKybConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg( KYB_REG_VAL_CFG_FILES_ROOT));
		return false;
	}

	m_sCfgFilesRoot = QString(szTemp);

//-----------------------------------------
// thread priorities
	//	QThread::Priority m_eAniThreadPriority; 
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		KYB_REG_VAL_ANI_THREAD_PRIORITY, 
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MKybConfigGeneral::loadConfigFromRegistry: Error key %1[%2]. Default QThread::NormalPriority (3) is used!")
											.arg(pcRegKey).arg(KYB_REG_VAL_ANI_THREAD_PRIORITY));
		m_eAniThreadPriority = QThread::NormalPriority; 
	}
	else
	{
		if(dwTemp<0||dwTemp>7) 
				m_eAniThreadPriority = QThread::NormalPriority;
		else
			m_eAniThreadPriority = (QThread::Priority)dwTemp;
	}
	
//---------------------------------------------
	if ((dwRes = REG_Lire_Entier (
		CSR_REG_KEYi_ROOT, 
		pcRegKey,
		REG_VAL_MAX_TRACE_SIZE_MB, 
		&dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MKybConfigGeneral::loadConfigFromRegistry: Error key %1[%2]! Using default 1!")
			.arg(pcRegKey).arg( REG_VAL_MAX_TRACE_SIZE_MB));
	}
	else
		MTracer::getTracer()->setMaxTraceSize(dwTemp);

	// 		bool	m_bHideCursor;
	if ((dwRes = REG_Lire_Entier(
		CSR_REG_KEYi_ROOT,
		pcRegKey,
		IHM_REG_VAL_HIDE_CURSOR,
		&dwTemp)) != ERROR_SUCCESS)
	{
		TRACE_W(QString("MIhmConfigGeneral::loadConfigFromRegistry: Error key %1[%2]").arg(pcRegKey).arg(IHM_REG_VAL_HIDE_CURSOR));
		m_bHideCursor = false;
	}
	else
	{
		m_bHideCursor = (dwTemp == 0) ? false : true;
	}


	
	return loadKeyboardConfigurations();;
}

MKybTypeSettings * MKybConfigGeneral::getKybTypeSettings(QString sCfgId)
{
	for(int i=0 ;i<m_lstKybSettings.size();i++)
	{
		if(QString::compare(m_lstKybSettings.at(i)->getCfgId(), sCfgId, Qt::CaseInsensitive) == 0)
			return m_lstKybSettings.at(i);
	}

	return NULL;
}


bool MKybConfigGeneral::loadKeyboardConfigurations()
{
	QString sRegSubKey;
	QString sRegBaseKey;
	bool bRetValue = true;

	sRegBaseKey = m_sModuleConfigKey + QString("\\Configurations");
	QSettings regBaseValuesIHM(sRegBaseKey, QSettings::NativeFormat);
	
	QStringList sList = regBaseValuesIHM.allKeys();

	for(int i=0;i<sList.size();i++)
	{
		QString sKeyId = sList.at(i);
		QString sFileName = regBaseValuesIHM.value(sKeyId).toString();
		
		MKybTypeSettings * pNewConfig = new MKybTypeSettings();
		bRetValue = bRetValue && pNewConfig->loadKeybSettings(sKeyId, getCfgFilesRoot() + sFileName);

		m_lstKybSettings.append(pNewConfig);
	}	

	if(sList.size()>0)
		return bRetValue;
	else 
	{
        TRACE_W(QString( "MKybConfigGeneral::loadKeyboardConfigurations: No configuration found in [%1]!")
			.arg(sRegBaseKey));
		return false;
	}	
}


QString MKybConfigGeneral::getCleanPath(QString sPath, QString sSubPath)
{
	QString sRet;

	sPath = sPath.trimmed();

	if(sPath.size()>0)
		if(sPath.right(1) == QString("/") || sPath.right(1) == QString("\\"))
			sPath = sPath.left(sPath.size()-1);
	
	if(sSubPath!="")
		sRet = QDir::cleanPath(sPath + QDir::separator() + sSubPath); 

	return sRet;
}


