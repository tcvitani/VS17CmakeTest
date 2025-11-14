
#include <QStringList>
#include "MHelpFuncs.h"


extern "C" {
	#include <ihm.H>
	#include <reg.h>
};

#include "MStrDetectionPair.h"
#include "MTracer.h"

#define IHM_REG_VAL_PREAMBLE		"Preamble"
#define IHM_REG_VAL_POSTAMBLE		"Postamble"
#define IHM_REG_VAL_ENDING_ASCII	"EndingAscii"

#define IHM_ESC_CHAR_ASCII_CODE '\\'
#define IHM_ESC_CHAR_SCAN_CODE '~'


MStrDetectionPair::MStrDetectionPair()
{
	m_bEnvoiePrePostambule = false;
	m_bActive = false;

}

MStrDetectionPair::MStrDetectionPair(QString sPairName)
{
	m_bEnvoiePrePostambule = false;
	m_bActive = false;
	m_sPairName = sPairName;
}


MStrDetectionPair::~MStrDetectionPair()
{
	while (!m_lstPreambleKeys.isEmpty())
		delete m_lstPreambleKeys.takeFirst();

	while (!m_lstPostambleKeys.isEmpty())
		delete m_lstPostambleKeys.takeFirst();


}


bool MStrDetectionPair::initialize(QString sConfRegistryPath)
{

    DWORD dwValeurLen;
    DWORD dwRes;
    char szTemp[IHM_LG_LIGNE_MAX];
    char pcRegKey[IHM_LG_LIGNE_MAX];

	strcpy(pcRegKey, sConfRegistryPath.toLatin1().data());

	dwValeurLen = sizeof (szTemp) ;
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_PREAMBLE, 
                        szTemp, 
                        &dwValeurLen )) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MStrDetectionPair::initialize: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_PREAMBLE));
		return false;
	}

	m_sPreamble = szTemp;

	extractPreamble();

	dwValeurLen = sizeof (szTemp) ;
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_POSTAMBLE, 
                        szTemp, 
                        &dwValeurLen )) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MStrDetectionPair::initialize: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_POSTAMBLE));
		return false;
	}

	m_sPostamble = szTemp;
	
	extractPostamble();

	dwValeurLen = sizeof (szTemp) ;
	if ((dwRes = REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)IHM_REG_VAL_ENDING_ASCII, 
                        szTemp, 
                        &dwValeurLen )) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MStrDetectionPair::initialize: Error key %1[%2]").arg(pcRegKey).arg( IHM_REG_VAL_ENDING_ASCII));
		return false;
	}

	m_sEndingAscii = szTemp;

	
	return true;
}


void MStrDetectionPair::extractPreamble()
{
	while (!m_lstPreambleKeys.isEmpty())
		delete m_lstPreambleKeys.takeFirst();

	QStringList sLstPreamble = m_sPreamble.split(IHM_SEPARATOR_CHAR);

	if(sLstPreamble.size()>0)
	{
		for(int i=0;i<sLstPreamble.size();i++)
		{
			QString sCharDesc = sLstPreamble.at(i);
			
			MEnteredKey *pNewKey = getEnteredKeyFromDesc(sCharDesc);	
			
			if(pNewKey!=NULL)
			{
				m_lstPreambleKeys.append(pNewKey);
			}
		}
	}

}




MEnteredKey * MStrDetectionPair::getEnteredKeyFromDesc(QString sCharDesc)
{
	MEnteredKey * pRetVal = NULL;

	if(sCharDesc.size()>1)
	{
		QChar ch = sCharDesc.at(0);
		QString sCode =  sCharDesc.mid(1);

		bool bOK;
		int iCode = sCode.toInt(&bOK);

		if(!(iCode>=0 && iCode<256))
				bOK = false;

		if( ch.toLatin1() == IHM_ESC_CHAR_ASCII_CODE && bOK)
		{
			pRetVal = new MEnteredKey();
			pRetVal->m_sText = QChar(iCode);
			pRetVal->m_iScanCode = 0;
			pRetVal->m_cAscii = (char)iCode;
			
		}
		else if( ch.toLatin1() == IHM_ESC_CHAR_SCAN_CODE && bOK)
		{
			pRetVal = new MEnteredKey();
			pRetVal->m_sText = "";
			pRetVal->m_iScanCode = iCode;
			pRetVal->m_cAscii = 0;

		}
		else
		{
			QString sMsg = QString("MStrDetectionPair::getNewCharFromDesc: Error decoding chars %1 for string detection pair %2!")
								.arg(sCharDesc).arg(m_sPairName);
			sMsg += QString("The chars should be separated with [|] char! \n");
			sMsg += QString("Each can be defined as ascii code i.e. \\\\128 or scan code i.e.~128, or by character itself i.e.*!");
			TRACE_W(sMsg);
		}
	}
	else if(sCharDesc.size()==1)
	{
		pRetVal = new MEnteredKey();
		pRetVal->m_sText = sCharDesc;
		pRetVal->m_iScanCode = 0;
		pRetVal->m_cAscii = (char)sCharDesc.at(0).toLatin1();
		
	}


	return pRetVal;
}


void MStrDetectionPair::extractPostamble()
{

	while (!m_lstPostambleKeys.isEmpty())
		delete m_lstPostambleKeys.takeFirst();

	QStringList sLstPostamble = m_sPostamble.split(IHM_SEPARATOR_CHAR);

	if(sLstPostamble.size()>0)
	{
		for(int i=0;i<sLstPostamble.size();i++)
		{
			QString sCharDesc = sLstPostamble.at(i);
			
			MEnteredKey *pNewKey = getEnteredKeyFromDesc(sCharDesc);	
			
			if(pNewKey!=NULL)
			{
				m_lstPostambleKeys.append(pNewKey);
			}
		}
	}
}



bool MStrDetectionPair::isPreambleKeyMatch(MEnteredKey *pKey, int iIndex)
{
	MEnteredKey *pCurrentKey;

	if(iIndex < m_lstPreambleKeys.size())
	{
		pCurrentKey = m_lstPreambleKeys.at(iIndex);
		
		return (*pKey == *pCurrentKey);
	}
	else
	{
		return true;
	}
}


int MStrDetectionPair::getNumPreambleKeys()
{
	return m_lstPreambleKeys.size();
}

int MStrDetectionPair::getNumPostambleKeys()
{
	return m_lstPostambleKeys.size();
}


bool MStrDetectionPair::isPostambleMatch(QList <MEnteredKey *> * pLstEnteredKeys)
{
	MEnteredKey *pCurrentKey;
	MEnteredKey *pCurrentPressedKey;
	
	int iPostambleSize = m_lstPostambleKeys.size();
	bool bMatch = false;
	int i = iPostambleSize-1;

	if(pLstEnteredKeys->size() > iPostambleSize + 3)
	{

		while(i>=0)
		{
			pCurrentKey = m_lstPostambleKeys.at(i);
			
			pCurrentPressedKey = pLstEnteredKeys->at(pLstEnteredKeys->size() - iPostambleSize + i);

			if(*pCurrentKey==*pCurrentPressedKey)
				bMatch = true;
			else
			{
				bMatch = false;
				break;
			}
			
			i--;
		}

	}


	return bMatch;
		
}

MStrDetectionPair& MStrDetectionPair::operator=(const MStrDetectionPair& right)
{
	this->m_sPairName = MHelpFuncs::deepCopy(right.m_sPairName);
	this->m_bEnvoiePrePostambule = right.m_bEnvoiePrePostambule;
	this->m_bActive = right.m_bActive;
		
	return *this;
}

