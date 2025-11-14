
#include <QStringList>
#include <QDateTime>
#include <QDomDocument>
#include <QTextStream>

#include "MInputDialogExReq.h"
#include "MInputExField.h"
#include "MHelpFuncs.h"
#include "MIhmConfigGeneral.h"
#include "MTracer.h"


extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
	#include <reg.h>
}



#define IHM_REG_VAL_PARAM_OPTION_EMBEDED   "EMBEDED"


MInputDialogExReq::MInputDialogExReq(MInputDialogExReq &obj)
{
	*this = obj;
}

MInputDialogExReq::MInputDialogExReq()
		:m_bEmbedded(false)
{
	m_iPosX = 0;
	m_iPosY = 0;
	m_eDialogType = enuSAISIE_EX_UNKNOWN;
	m_retBalId = 0;
	
	m_bHasOKButton = true;
	m_bHasCancelButton = true;

	m_iVirtKybPosX = -1;
	m_iVirtKybPosY = -1;
}


MInputDialogExReq::~MInputDialogExReq()
{
}

	
void MInputDialogExReq::generateRequestID()
{
	m_sDlgReqId = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
}



void MInputDialogExReq::init(QString sModuleCfgKey, QString sDialogID)
{
	bool bOK;
	generateRequestID();

	QString sRegDef = MInputDialogExReq::readInputDialogDefinition(sModuleCfgKey, sDialogID);
	
	if(sRegDef=="")
	{
		TRACE_W(QString("MInputDialogExReq::init:Registry definition missing for dialog id:%1")
									.arg(sDialogID));
		m_bValid = false;
		return;
	}
	
	m_sDialogId = sDialogID;

	//extracting dialog type
	QString sDialog_Type = MHelpFuncs::getStringItem(sRegDef, 0, QString(IHM_SAISIE_EX_SEPARATEUR_CHAMP), true);
	int iDlgType =  sDialog_Type.toInt(&bOK);

	if(bOK && iDlgType <= enuSAISIE_EX_SEL_LAST_TYPE && iDlgType > enuSAISIE_EX_UNKNOWN)
	{
		m_eDialogType = (enuDialogExType)iDlgType;
	}
	else
	{
		TRACE_W(QString("MInputDialogExReq::init: sDialogID %1 - invalid type %2")
									.arg(sDialogID)
									.arg(iDlgType));
		m_bValid = false;
		return;
	}

	//extracting dialog position
	QString sDialog_Pos = MHelpFuncs::getStringItem(sRegDef, 1, QString(IHM_SAISIE_EX_SEPARATEUR_CHAMP), true);

	QString sParam1 = MHelpFuncs::getStringItem(sDialog_Pos, 0, QString(IHM_SAISIE_EX_SEPARATEUR_PARAM), true);
	QString sParam2 = MHelpFuncs::getStringItem(sDialog_Pos, 1, QString(IHM_SAISIE_EX_SEPARATEUR_PARAM), true);

	if(sParam1==IHM_REG_VAL_PARAM_OPTION_EMBEDED)
	{
		m_bEmbedded = true;
		m_sRelativeToObjectName = sParam2;
	}
	else
	{
		m_iPosX = sParam1.toInt(&bOK);

		if(bOK)
			m_iPosY = sParam2.toInt(&bOK);

		if(!bOK)
		{
			TRACE_W(QString("MInputDialogExReq::init: sDialogID %1 - invalid position %2")
									.arg(sDialogID)
									.arg(sDialog_Pos));
			m_bValid = false;
			return;

		}
	}


	if (m_eDialogType == enuSAISIE_EX_CUSTOM_PLUGIN_DLG)
	{
		m_sCustomDlg_PluginName = MHelpFuncs::getStringItem(sRegDef, 2, QString(IHM_SAISIE_EX_SEPARATEUR_CHAMP), true);
		if(m_sCustomDlg_PluginName == "")
		{
			TRACE_W(QString("MInputDialogExReq::init:  sDialogID [%1] - invalid PluginName in [%2]")
				.arg(sDialogID).arg(sRegDef));
			m_bValid = false;
			return;
		}

		QString sCustomDlg_DlgId = MHelpFuncs::getStringItem(sRegDef, 3, QString(IHM_SAISIE_EX_SEPARATEUR_CHAMP), true);
		bool bOK;
		m_iCustomDlg_DlgId = sCustomDlg_DlgId.trimmed().toInt(&bOK);

		if (sCustomDlg_DlgId == "" || !bOK || m_iCustomDlg_DlgId <= 0)
		{
			TRACE_W(QString("MInputDialogExReq::init: sDialogID %1 - invalid DlgId")
				.arg(sCustomDlg_DlgId));
			m_bValid = false;
			return;
		}


		//m_sCustomDlg_InitialParameters = MHelpFuncs::getStringItem(sRegDef, 4, QString(IHM_SAISIE_EX_SEPARATEUR_CHAMP), true);

	}
	else
	{
		//extracting desktop template file 
		m_sDskUITemplate = MHelpFuncs::getStringItem(sRegDef, 2, QString(IHM_SAISIE_EX_SEPARATEUR_CHAMP), true);

		if (m_sDskUITemplate == "")
		{
			TRACE_W(QString("MInputDialogExReq::init: sDialogID %1 - invalid desktop ui template  %2")
				.arg(sDialogID)
				.arg(m_sDskUITemplate));
			m_bValid = false;
			return;
		}



		//extracting web template file 
		m_sWebUITemplate = MHelpFuncs::getStringItem(sRegDef, 3, QString(IHM_SAISIE_EX_SEPARATEUR_CHAMP), true);

		if (m_sWebUITemplate == "")
		{
			TRACE_W(QString("MInputDialogExReq::init: sDialogID %1 - invalid web ui template  %2")
				.arg(sDialogID)
				.arg(m_sWebUITemplate));
			m_bValid = false;
			return;
		}

		//extracting dialog def file (list of widgets its types and possibly input format)
		QString sDefFile = MHelpFuncs::getStringItem(sRegDef, 4, QString(IHM_SAISIE_EX_SEPARATEUR_CHAMP), true);


		if (!setDefinition(sDefFile))
		{
			TRACE_W(QString("MInputDialogExReq::init: sDialogID %1 - invalid definition file  %2")
				.arg(sDialogID)
				.arg(sDefFile));
			m_bValid = false;
			return;
		}
	}
	
	//extracting virtual keyboard configuration
	QString sVirtKybDef = MHelpFuncs::getStringItem(sRegDef, 5, QString(IHM_SAISIE_EX_SEPARATEUR_CHAMP), true);
	
	sVirtKybDef = sVirtKybDef.trimmed();

	if(!sVirtKybDef.isEmpty())
	{
		QString sParam1 = MHelpFuncs::getStringItem(sVirtKybDef, 0, QString(IHM_SAISIE_EX_SEPARATEUR_PARAM), true);
		QString sParam2 = MHelpFuncs::getStringItem(sVirtKybDef, 1, QString(IHM_SAISIE_EX_SEPARATEUR_PARAM), true);
		QString sParam3= MHelpFuncs::getStringItem(sVirtKybDef, 2, QString(IHM_SAISIE_EX_SEPARATEUR_PARAM), true);

		m_sVirtKeybConfigToUse = sParam1;
		m_iVirtKybPosX = sParam2.toInt();
		m_iVirtKybPosY = sParam3.toInt();
	}
	

	m_bValid = true;

}


bool MInputDialogExReq::hasOKButton()
{
	return m_bHasOKButton;
}

bool MInputDialogExReq::hasCancelButton()
{
	return m_bHasCancelButton;
}

void MInputDialogExReq::setRetBalId(unsigned long retBalId)
{
	m_retBalId = retBalId;
}

bool MInputDialogExReq::operator== ( const MInputDialogExReq & right ) const
{
	return (m_sDlgReqId == right.m_sDlgReqId);
}	

MInputDialogExReq& MInputDialogExReq::operator=(const MInputDialogExReq& right)
{

	this->m_bEmbedded = right.m_bEmbedded;
	this->m_sRelativeToObjectName = MHelpFuncs::deepCopy(right.m_sRelativeToObjectName);

	//if not embedded to an object
	this->m_iPosX = right.m_iPosX;
	this->m_iPosY = right.m_iPosY;

	this->m_eDialogType = right.m_eDialogType;
	this->m_retBalId = right.m_retBalId;
	
	this->m_bHasOKButton = right.m_bHasOKButton;
	this->m_bHasCancelButton = right.m_bHasCancelButton;
	
	this->m_sDescription = MHelpFuncs::deepCopy(right.m_sDescription);
	this->m_sDefinition = MHelpFuncs::deepCopy(right.m_sDefinition);

	this->m_sDialogId = MHelpFuncs::deepCopy(right.m_sDialogId);
	this->m_sDskUITemplate = MHelpFuncs::deepCopy(right.m_sDskUITemplate);
	this->m_sWebUITemplate = MHelpFuncs::deepCopy(right.m_sWebUITemplate);
	this->m_sDskCSSFile = MHelpFuncs::deepCopy(right.m_sDskCSSFile);
	
	while (!m_lstInputFields.isEmpty())
		delete m_lstInputFields.takeFirst();

	MInputExField * pCurrent;

	for(int i=0;i<right.m_lstInputFields.size();i++)
	{
		pCurrent = right.m_lstInputFields.at(i);

		if(pCurrent!=NULL)
		{
			MInputExField * pNew = new MInputExField();
			*pNew = *pCurrent;
			m_lstInputFields.append(pNew);
		}

	}

	this->m_bValid = right.m_bValid;
	this->m_sDlgReqId = MHelpFuncs::deepCopy(right.m_sDlgReqId);

	this->m_sVirtKeybConfigToUse = MHelpFuncs::deepCopy(right.m_sVirtKeybConfigToUse);
	this->m_iVirtKybPosX = right.m_iVirtKybPosX;
	this->m_iVirtKybPosY = right.m_iVirtKybPosY;

	this->m_sCustomDlg_PluginName = right.m_sCustomDlg_PluginName;
	this->m_sCustomDlg_Parameters = right.m_sCustomDlg_Parameters;
	this->m_iCustomDlg_DlgId = right.m_iCustomDlg_DlgId;

	return *this;	
	
}



QString MInputDialogExReq::readInputDialogDefinition(QString sModuleCfgKey, QString sDialogID)
{

	QString sRes = "";
    DWORD dwValeurLen;
    DWORD dwRes;
    char pcRegKey[IHM_LG_LIGNE_MAX];
    char szWork[IHM_LG_LIGNE_MAX];

	QString sKey = sModuleCfgKey + QString("\\InputEx");
	strcpy_s(pcRegKey, sizeof(pcRegKey), sKey.toLatin1().data());

	// Pool utilise : string
	// 	QString m_sPool;
	dwValeurLen = sizeof (szWork) ;
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)sDialogID.toLatin1().data(),
                        szWork, 
                        (unsigned long*)&dwValeurLen ) ) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MInputDialogExReq::readInputDialogDefinition: Erreur cle %1[%2]")
									.arg(pcRegKey)
									.arg(sDialogID));
	}
	else
		sRes = szWork;

	return sRes;
}	


bool MInputDialogExReq::setCustomDlg_Parameters(QString sParams, int is_file)
{

	if (sParams.size() > 0)
	{
		if (is_file != 0) // check if it is file name
		{
			QString sDesFileName = sParams;

			QString sFullPath = MIhmConfigGeneral::getCfg()->getCommFileFullPath(sDesFileName);

			TRACE_D(QString("MInputDialogExReq::setCustomDlg_Parameters: File %1 !").arg(sFullPath));
			//read the file content
			m_sCustomDlg_Parameters = MHelpFuncs::loadFileContent(sFullPath);

			if (m_sCustomDlg_Parameters == "")
			{
				TRACE_W(QString("MInputDialogExReq::setCustomDlg_Parameters: Error in file %1!").arg(sFullPath));
				
				m_bValid = false;
				return false;
			}

		}
		else
			m_sCustomDlg_Parameters = sParams;
	}

	return true;
}