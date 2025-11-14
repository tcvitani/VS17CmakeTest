

#include <QStringList.h>
#include "MIhmMsg.h"

#include "MStringDetection.h"
#include "MStrDetectionPair.h"
#include "MStringDetectionAtm.h"
#include "MTracer.h"


extern "C" {
	#include <ihm.h>
	#include <reg.h>
}


#define IHM_TOLL_KEY_DETECTION_PAIR_NAME "TollKey"



MStringDetection::MStringDetection()
{
	m_pTollKeyDetectionPair = new MStrDetectionPair(IHM_TOLL_KEY_DETECTION_PAIR_NAME);
	m_pDetectionPairMatch = NULL;

	m_pStrDetAtm = new MStringDetectionAtm();
	
	m_pStrDetAtm->initialize(this);
}



MStringDetection::~MStringDetection()
{
	delete m_pTollKeyDetectionPair;
	delete m_pStrDetAtm;

	while(!m_lstPairs.isEmpty())
		delete m_lstPairs.takeLast();
}

bool MStringDetection::getCompleteStringDetection(MIhmMsgStringDetection * pMsg, bool * pbPairActuallyActive)
{
	bool bRet = false;

	*pbPairActuallyActive = false;

	if(m_pDetectionPairMatch!=NULL)
	{
		*pbPairActuallyActive = m_pDetectionPairMatch->isActive();

		pMsg->sCoupleId = m_pDetectionPairMatch->getName();
		pMsg->bStringComplete = true;
		pMsg->bEnvoiePrePostambule = m_pDetectionPairMatch->m_bEnvoiePrePostambule;
		QString sDetectedString = getDetectedString();
		
		sDetectedString.append(m_pDetectionPairMatch->getEndingAscii());
		
		pMsg->sDetectedString = sDetectedString;
		bRet = true;
	}

	clearInputBuffer();

	return bRet;
}

int MStringDetection::getTollKeyDetection()
{

	int iCode = 0;

	if(m_pDetectionPairMatch!=NULL)
	{
		QString sDetectedString = getDetectedString();

		bool bOK;
		iCode = sDetectedString.toInt(&bOK);

		if(!bOK)
			iCode = 0;
	}
	
	clearInputBuffer();

	return iCode;
}

QString  MStringDetection::getCompleteStringDetectionWithPrePostAmbules()
{
	QString sRetVal;

	if(m_pDetectionPairMatch!=NULL)	
	{
		MEnteredKey * pEnteredKey;
		
		for(int j = 0; j < m_lstEnteredKeys.size(); j++) 
		{
			pEnteredKey = m_lstEnteredKeys.at(j);
			sRetVal += pEnteredKey->m_sText;
		}
	}

	return sRetVal;
}



bool MStringDetection::isTollKey()
{
	if(m_pDetectionPairMatch == m_pTollKeyDetectionPair)
		return true;
	else 
		return false;
}

bool MStringDetection::isStringComplete()
{
	return 	m_bDetectionCompleteSucces;
}


void MStringDetection::detectionComplete()
{
	m_bDetectionCompleteSucces = true;
}

//This one takes care about sending or not the pre and postamble keys
QString MStringDetection::getDetectedString()
{
	QString sRetVal;

	MEnteredKey * pEnteredKey;
	
	for(int j = 0; j < m_lstEnteredKeys.size(); j++) 
	{
		pEnteredKey = m_lstEnteredKeys.at(j);
		
		if(m_pDetectionPairMatch->m_bEnvoiePrePostambule)
		{	
			sRetVal += pEnteredKey->m_sText;
		}
		else if(j>m_pDetectionPairMatch->getNumPreambleKeys()-1 && 
			j<m_lstEnteredKeys.size()-m_pDetectionPairMatch->getNumPostambleKeys())
		{
			sRetVal += pEnteredKey->m_sText;
		}
	}


	return sRetVal;

}

void MStringDetection::resetPairs()
{

	while (!m_lstPairs.isEmpty())
		delete m_lstPairs.takeFirst();

	clearInputBuffer();

}

void MStringDetection::initialize(QString sModConfigKey)
{
   	resetPairs();
	
	//activate tolling keys detection
	QString sConfigKeyRoot = sModConfigKey + QString("\\%1").arg(IHM_REG_KEY_STRING_DETECTION);
	m_pTollKeyDetectionPair->initialize(sConfigKeyRoot);
	m_pTollKeyDetectionPair->m_bActive = true;

	//load dynamically configurable string detection pairs 
	loadDetectionPairs(sModConfigKey, &m_lstPairs);
	loadDetectionTimeout(sModConfigKey);

	QObject::connect(&m_tmDetectionTimer, SIGNAL(timeout()), this, SLOT(onDetectionTimerTimeout()));
}


bool MStringDetection::loadDetectionPairs(IN QString sModConfigKey, OUT QList <MStrDetectionPair *> *pLst)
{
	bool bRetVal = false;
	DWORD dwIndex = 0;
	char pcNomCle[MAX_PATH];
	DWORD dwTailleCle = MAX_PATH;
	char pcRegKey[MAX_PATH];
	
	QString sConfigKey = sModConfigKey + QString("\\%1").arg(IHM_REG_KEY_STRING_DETECTION);
	strcpy(pcRegKey,sConfigKey.toLatin1().data());

	DWORD dwRetVal =  REG_Enum_Cles(
                      CSR_REG_KEYi_ROOT,
                      pcRegKey,
                      dwIndex,
                      pcNomCle,
                      &dwTailleCle);   
     
    while(dwRetVal == ERROR_SUCCESS)
    {
		
        if (dwTailleCle!=0)
        {
			MStrDetectionPair * pNewPair;
			pNewPair = new MStrDetectionPair(pcNomCle);
			QString sChildKey = sConfigKey + QString("\\%1").arg(pcNomCle);

			if( pNewPair->initialize(sChildKey))
				pLst->append(pNewPair);
			else
				delete pNewPair;
        }
         
        // Enumerer la valeur suivante
        dwIndex = dwIndex + 1;
		dwTailleCle = MAX_PATH;

        dwRetVal = REG_Enum_Cles(CSR_REG_KEYi_ROOT,
                      pcRegKey,
                      dwIndex,
                      pcNomCle,
                      &dwTailleCle);
    }

	return bRetVal;
}

bool MStringDetection::loadDetectionTimeout(IN QString sModConfigKey)
{
	bool bRetVal = false;
	DWORD dwTimeout = 0;
	char pcRegKey[MAX_PATH] = { 0 };
	
	QString sConfigKey = sModConfigKey + QString("\\%1").arg(IHM_REG_KEY_STRING_DETECTION);
	
	strcpy_s(pcRegKey, sizeof(pcRegKey), sConfigKey.toLatin1().data());
	
	DWORD dwRetVal = REG_Lire_Entier(CSR_REG_KEYi_ROOT,pcRegKey, (char*)"StringDetectionTimeOut_ms", &dwTimeout);
	if(dwRetVal == ERROR_SUCCESS)
    {
		m_iDetectionTimeOut = dwTimeout;
    }
	
	return bRetVal;
}



void MStringDetection::updateStrDetectionPairs(QList <MStrDetectionPair *> *lst)
{

	MStrDetectionPair *pCurrentVirtualPair;
	MStrDetectionPair *pCurrentPair;

	deactivateAllPairs();

	for(int i = 0;i < lst->size(); i++)
	{
		pCurrentVirtualPair = lst->at(i);

		for(int j = 0; j < m_lstPairs.size(); j++)
		{
			pCurrentPair = m_lstPairs.at(j);

			if(pCurrentPair->getName() == pCurrentVirtualPair->getName())
			{
				//update the data
				pCurrentPair->m_bActive = pCurrentVirtualPair->m_bActive;
				pCurrentPair->m_bEnvoiePrePostambule = pCurrentVirtualPair->m_bEnvoiePrePostambule;
				break;
			}
		}
		
	}

}

void MStringDetection::addNewKeyToBuffer(QString sText, int iScanCode)
{
	MEnteredKey *pNewKey = new MEnteredKey();
	pNewKey->m_sText = sText;
	pNewKey->m_iScanCode = iScanCode;

	if(sText.size()>0) 
		pNewKey->m_cAscii = (char)sText.at(0).toLatin1();

	m_lstEnteredKeys.append(pNewKey);

}

void MStringDetection::clearInputBuffer()
{
	while(!m_lstEnteredKeys.isEmpty())
		delete m_lstEnteredKeys.takeFirst();

	m_pDetectionPairMatch = NULL;
	m_bDetectionCompleteSucces = false;

}

bool MStringDetection::checkIfStringDetection(QString sText, int iScanCode)
{

	//add the new key to the input buffer
	addNewKeyToBuffer(sText,iScanCode);	

	MStrDetAtmEvent *pEvent = new MStrDetAtmEvent(MStringDetectionAtm::enuEvtNewKey);
	
	//this one will process the key, update the input buffer and send response if necessary
	m_pStrDetAtm->processEvent(pEvent);

	if(m_pStrDetAtm->getCurrentStateId() == MStringDetectionAtm::enuNoFrameReading
		&& !m_bDetectionCompleteSucces)
	{
		//frame reading is not in progress
		return false;
	}
	else
	{
		//string detected by now
		return true;
	}

}

int MStringDetection::getCountPreambleMatch(bool *pbCompleteMatch)
{
	int iCount = 0;

	MStrDetectionPair * pDetPair;
	MStrDetectionPair * pLastGoodPair;
	bool bGoodPairCompleteMatch;
	bool bCurrentPairCompleteMatch;

	m_pDetectionPairMatch = NULL;

	for(int i = 0; i < m_lstPairs.size(); i++)
	{
		pDetPair = m_lstPairs.at(i);
		
		//if(pDetPair->isActive())
		if(isPreamblePairMatch(pDetPair, &bCurrentPairCompleteMatch))
		{
			bGoodPairCompleteMatch = bCurrentPairCompleteMatch;
			pLastGoodPair = pDetPair;
			iCount++; //one more good pair
		}
	}

	//also verify if it is the toll key  (m_pTollKeyDetectionPair)
	if(isPreamblePairMatch(m_pTollKeyDetectionPair, &bCurrentPairCompleteMatch))
	{
		bGoodPairCompleteMatch = bCurrentPairCompleteMatch;
		pLastGoodPair = m_pTollKeyDetectionPair;
		iCount++; //one more good pair
	}



	if(iCount==1 && bGoodPairCompleteMatch)
	{
		//only one pair matches sequence in the buffer
		//if it is complete PreamblePairMatch
		*pbCompleteMatch = true;
		m_pDetectionPairMatch = pLastGoodPair;
		TRACE_D(QString("MStringDetection::getCountPreambleMatch: Selected matching pair %1!").arg(m_pDetectionPairMatch->getName()));
	}
	else
		*pbCompleteMatch = false;

	return iCount;
}



bool MStringDetection::isPreamblePairMatch(MStrDetectionPair * pDetPair, bool *pbFullMatch)
{
	MEnteredKey * pEnteredKey;
	*pbFullMatch = false;
	bool bMatch = false;

	for(int j = 0; j < m_lstEnteredKeys.size(); j++) 
	{
		pEnteredKey = m_lstEnteredKeys.at(j);

		bMatch = pDetPair->isPreambleKeyMatch(pEnteredKey,j);
		
		if(!bMatch)
			return false;
	}


	if(m_lstEnteredKeys.size()>=pDetPair->getNumPreambleKeys())
	{
		*pbFullMatch = true;
	}


	return bMatch;
}



bool MStringDetection::isPostambleMatch(bool * pbInputBufferOK)
{
	*pbInputBufferOK = true;
	

	if(m_pDetectionPairMatch!=NULL)
	{
		if(m_lstEnteredKeys.size()>0)
		{
			return m_pDetectionPairMatch->isPostambleMatch(&m_lstEnteredKeys);
		}
		else
		{
			TRACE_W("MStringDetection::isPostambleMatch: ERROR m_lstEnteredKeys size == 0");
			*pbInputBufferOK = false;
			return false;
		}

	}
	else
	{
		TRACE_W("MStringDetection::isPostambleMatch: ERROR m_pDetectionPairMatch == NULL");
		*pbInputBufferOK = false;
		return false;
	}
}


void MStringDetection::startTimer()
{
	m_tmDetectionTimer.start(m_iDetectionTimeOut);
}


void MStringDetection::stopTimer()
{
	m_tmDetectionTimer.stop();
}


void MStringDetection::onDetectionTimerTimeout()
{
	TRACE_D("MStringDetection::onDetectionTimerTimeout");
	
	m_pStrDetAtm->timeout();
}



void MStringDetection::activateAllPairs()
{
	MStrDetectionPair *pCurrentPair;

	for(int j = 0; j < m_lstPairs.size(); j++)
	{
		pCurrentPair = m_lstPairs.at(j);
		pCurrentPair->m_bActive = true;
		pCurrentPair->m_bEnvoiePrePostambule = true;
	}
}

void MStringDetection::deactivateAllPairs()
{
	for(int j = 0; j < m_lstPairs.size(); j++)
	{
		m_lstPairs.at(j)->m_bActive = false;
	}
}