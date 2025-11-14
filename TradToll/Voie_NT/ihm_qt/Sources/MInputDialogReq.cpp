
#include <QStringList>
#include <QDateTime>
#include <MInputDialogReq.h>
#include <MHelpFuncs.h>
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
	#include <reg.h>
}

const char MInputDialogReq::cSAISIE_VEROU = '!';
const char MInputDialogReq::cSAISIE_DECODE_TEXTE = '?';
const char MInputDialogReq::cSAISIE_RAZ_ZONE_SAISIE = '^';
const char MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP = '|';
const char MInputDialogReq::cSAISIE_SEPARATEUR_PARAM = '~';
const char MInputDialogReq::cSAISIE_SEPARATEUR_ITEM = '&';

const char MInputDialogReq::cFORMAT_CHAR_NUMBER = 'N';
const char MInputDialogReq::cFORMAT_CHAR_STRING = 'C';
const char MInputDialogReq::cFORMAT_CHAR_MASK = 'M';
const char MInputDialogReq::cFORMAT_CHAR_RIGHTMASK = 'R';
const char MInputDialogReq::cFORMAT_CHAR_PASSWORD = '*';
const char MInputDialogReq::cFORMAT_CHAR_FUNCTION = 'F';



MInputDialogReq::MInputDialogReq(MInputDialogReq &obj)
{
	*this = obj;
}

MInputDialogReq &MInputDialogReq::operator=( const MInputDialogReq & right )	
{
	this->m_bEmbeded = right.m_bEmbeded;
	this->m_sRelativeToObjectName = MHelpFuncs::deepCopy(right.m_sRelativeToObjectName);
	
	this->m_flagsAligment = right.m_flagsAligment;

	this->m_sColorAttributes = MHelpFuncs::deepCopy(right.m_sColorAttributes);

	this->m_eInputType = right.m_eInputType;
	this->m_eDialogType = right.m_eDialogType;
	this->m_ucButtons = right.m_ucButtons;
	this->m_retBalId = right.m_retBalId;
	
	this->m_sDescription = MHelpFuncs::deepCopy(right.m_sDescription);
	this->m_sDefinition = MHelpFuncs::deepCopy(right.m_sDefinition);
	this->m_sDialogId = MHelpFuncs::deepCopy(right.m_sDialogId);
	this->m_sDialogTitle = MHelpFuncs::deepCopy(right.m_sDialogTitle);
	this->m_sUITemplate = MHelpFuncs::deepCopy(right.m_sUITemplate);
	this->m_lNbInputFields = right.m_lNbInputFields;

	this->m_bValid = right.m_bValid;
	this->m_bDefinitionOK = right.m_bDefinitionOK;

	//used to identify the request  
	this->m_sDlgReqId = MHelpFuncs::deepCopy(right.m_sDlgReqId);

	return *this;
}


void MInputDialogReq::generateRequestID()
{
	m_sDlgReqId = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
}


void MInputDialogReq::setDialogInputType(unsigned long dwInputType)
{
	m_eInputType =	(enuDialogInputType)dwInputType;

	generateRequestID();
}

void MInputDialogReq::setButtons(unsigned char buttons)
{
	m_ucButtons = buttons;

	if(buttons==0)
	{
		TRACE_W(QString("MInputDialogReq::setButtons: Invalid buttons! %1. Setting default to OK only.")
							.arg(buttons));
		m_ucButtons = ihmBoutonOK;
	}
}

bool MInputDialogReq::hasOKButton()
{
	return ((m_ucButtons & ihmBoutonOK) != 0);
}

bool MInputDialogReq::hasCancelButton()
{
	return ((m_ucButtons & ihmBoutonCANCEL) != 0);
}


void MInputDialogReq::setDescription(QString sDescription)
{
	if(sDescription.size()>0)
	{
		QString sPrefix = (sDescription.at(0) != IHM_SEPARATOR_CHAR ? IHM_SEPARATOR_STRING : ""); 
		m_sDescription = sPrefix;
		m_sDescription += sDescription;

		
		//parse description 
		if(parseDescription())
				m_bValid = true;
	}
	else
	{
		TRACE_W("MInputDialogReq::setDescription: Invalid empty description!");
		m_bValid = false;
	}
}

bool MInputDialogReq::parseDescription()
{
	QStringList sLstDescription = m_sDescription.split(cSAISIE_SEPARATEUR_CHAMP);

	if(m_sDescription=="" || 
		m_sDescription==QString(cSAISIE_SEPARATEUR_CHAMP) || 
		sLstDescription.size()<2)
		return false;

	m_sDialogId = sLstDescription.at(1);
	if(m_sDialogId=="")
			return false;
	
	// Recuperer le nombre de champs de saisie et verifier sa coherence
	QString sNumFields = sLstDescription.at(2);
	bool bOK;
	m_lNbInputFields = sNumFields.toInt(&bOK);
    
	if ((m_lNbInputFields > SAISIE_MAX_CHAMPS) || (m_lNbInputFields < 0) || !bOK) 
			return false;

	return true;
}

//
bool MInputDialogReq::verifyResponseFormat(QString sDlgResult, QString &sNewDescription)
{
	bool bOK = false;
	QStringList sLstResultFields;

	if(sDlgResult=="")
		bOK = true; 
	else
	{
		sLstResultFields = sDlgResult.split(cSAISIE_SEPARATEUR_CHAMP);
		
		if(sLstResultFields.size()>=2)
			if(sLstResultFields.at(1)==m_sDialogId)
			{
				if(m_eDialogType == enuSAISIE_CAT_FIELD)
				{
					if(sLstResultFields.size()==m_lNbInputFields+4)
					{	
						bool bAllFieldsOK = true;
						sNewDescription = cSAISIE_SEPARATEUR_CHAMP;
						
						for (int i=0; i<m_lNbInputFields;i++)
						{
							QString sFieldDef = getFieldDefinition(i);
							
							if(!verifyField(sFieldDef, sLstResultFields.at(i+3)))
							{
								sNewDescription += getFieldDescription(i);
								bAllFieldsOK = false;
							}
							else
							{
								QString sFieldData = sLstResultFields.at(i+3);
								QString sOldDesc = getFieldDescription(i);
								sNewDescription += replaceDescResult(sOldDesc, sFieldData);
							}

							sNewDescription += cSAISIE_SEPARATEUR_CHAMP;
						}

						if(bAllFieldsOK)
								bOK = true;
					}
				}
				else if(m_eDialogType == enuSAISIE_CAT_ALARM)
				{
					bOK = true;
				}
				else
				{
					if(sLstResultFields.size()==5)
					{
						bOK = true;
					}
				}
			}
	}

	if(sNewDescription=="")
			sNewDescription = m_sDescription;

	return bOK;
}

bool MInputDialogReq::verifyField(QString sFormat, QString sResult)
{
	bool bOK = true;
	return bOK;
}

QString MInputDialogReq::replaceDescResult(QString sOldDesc, QString sFieldData)
{
	QString sRes = sOldDesc;

	return sRes;
}

//the function is to be called after setDescription with correct 
// registry settings for the 
bool MInputDialogReq::setDefinition(QString &sDefinition)
{
	if(sDefinition.isEmpty())
		return false;

	if(!isValid()) //if description string is invalid return false
		return false;
	
	QString sPrefix = (sDefinition.at(0) != IHM_SEPARATOR_CHAR ? IHM_SEPARATOR_STRING : ""); 
	m_sDefinition = sPrefix;
	m_sDefinition += sDefinition;

	QStringList sLstDefinition = m_sDefinition.split(cSAISIE_SEPARATEUR_CHAMP);
	

	QString sTempTitle = sLstDefinition.at(1);
	m_sDialogTitle = MHelpFuncs::getStringItem(sTempTitle,0,QString(cSAISIE_SEPARATEUR_PARAM),true);
	m_sUITemplate = MHelpFuncs::getStringItem(sTempTitle,1,QString(cSAISIE_SEPARATEUR_PARAM),true);

	if(	m_sDescription == QString(cSAISIE_SEPARATEUR_CHAMP) || 
		sLstDefinition.size()<3)
			return false;


	QString sCatDef = sLstDefinition.at(2); //dialog category and col1,...col4 colors

	QString sCat = MHelpFuncs::getStringItem(sCatDef,0,QString(cSAISIE_SEPARATEUR_PARAM),true);
	m_sColorAttributes = MHelpFuncs::getStringItem(sCatDef,1,QString(cSAISIE_SEPARATEUR_PARAM),true);

	if(sCat == SAISIE_CAT_FIELD_TXT || sCat == SAISIE_CAT_FIELD_NUM )
		m_eDialogType = enuSAISIE_CAT_FIELD;
	else if(sCat == SAISIE_CAT_ALARM_TXT || sCat == SAISIE_CAT_ALARM_NUM )
		m_eDialogType = enuSAISIE_CAT_ALARM;
	else if(sCat == SAISIE_CAT_CHOICE_TXT || sCat == SAISIE_CAT_CHOICE_NUM )
		m_eDialogType = enuSAISIE_CAT_CHOICE;
	else if(sCat == SAISIE_CAT_BIGCHOICE_TXT || sCat == SAISIE_CAT_BIGCHOICE_NUM )
		m_eDialogType = enuSAISIE_CAT_BIGCHOICE;
	else
		return false;	

	//positioning parameters

	QString sAlignParam1 = sLstDefinition.at(3).toUpper(); 
	QString sAlignParam2 = sLstDefinition.at(4); 

	
	m_flagsAligment = (Qt::AlignmentFlag)0;
	
	if(sAlignParam1!="EMBEDED")
	{
		m_bEmbeded = false;
		sAlignParam2 = sAlignParam2.toUpper();
		
		if(sAlignParam1 == "LEFT")
			m_flagsAligment = Qt::AlignLeft;
		else if(sAlignParam1 == "CENTER")
			m_flagsAligment = Qt::AlignHCenter;
		else if(sAlignParam1 == "RIGHT")
			m_flagsAligment = Qt::AlignRight;
		else //default 
			m_flagsAligment = Qt::AlignHCenter;

		if(sAlignParam2 == "TOP")
			m_flagsAligment = m_flagsAligment | Qt::AlignTop;
		else if(sAlignParam2 == "CENTER")
			m_flagsAligment = m_flagsAligment | Qt::AlignVCenter;
		else if(sAlignParam2 == "BOTTOM")
			m_flagsAligment = m_flagsAligment | Qt::AlignBottom;
		else //default 
			m_flagsAligment = m_flagsAligment | Qt::AlignVCenter;

	}
	else
	{
		m_bEmbeded = true;
		m_sRelativeToObjectName = sAlignParam2;
	}
   

	m_bDefinitionOK = true;

	return m_bDefinitionOK;
	
}


QString MInputDialogReq::getDialogTitle()
{
	QString sRet;
	
	if(m_bDefinitionOK)
		sRet = m_sDialogTitle;

	return sRet;
}



void MInputDialogReq::setRetBalId(unsigned long retBalId)
{
	m_retBalId = retBalId;
}

bool MInputDialogReq::operator== ( const MInputDialogReq & right ) const
{
	bool bRetVal = false;

	bRetVal = (m_sDescription == right.m_sDescription);
	bRetVal = bRetVal && (m_eInputType == right.m_eInputType);
	bRetVal = bRetVal && (m_ucButtons == right.m_ucButtons);
	bRetVal = bRetVal && (m_retBalId == right.m_retBalId);
	bRetVal = bRetVal && (m_sDlgReqId == right.m_sDlgReqId);
	
	return bRetVal;
}	


QString MInputDialogReq::readInputDialogDefinition(QString sModuleCfgKey, QString sDialogID)
{

	QString sRes = "";
    DWORD dwValeurLen;
    DWORD dwRes;
    char pcRegKey[IHM_LG_LIGNE_MAX];
    char szWork[IHM_LG_LIGNE_MAX];

	QString sKey = sModuleCfgKey + QString("\\Input");
	strcpy(pcRegKey, sKey.toLatin1().data());

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
        TRACE_W(QString( "MInputDialogReq::readInputDialogDefinition: Erreur cle %1[%2]")
							.arg(pcRegKey)
							.arg(sDialogID));
	}
	else
		sRes = szWork;

	return sRes;
}	



QString MInputDialogReq::getFieldDescription(int iIndex)
{
	QStringList sLstDescription = m_sDescription.split(cSAISIE_SEPARATEUR_CHAMP);
	
	if(iIndex+3 < sLstDescription.size())
		return sLstDescription.at(iIndex+3);
	else
		return "";
}

QString MInputDialogReq::getFieldDefinition(int iIndex)
{
	QStringList sLstDefinition = m_sDefinition.split(cSAISIE_SEPARATEUR_CHAMP);
	
	if(iIndex+5 < sLstDefinition.size())
		return sLstDefinition.at(iIndex+5);
	else
		return "";

}



