
#include <MIhmSessionUserData.h>
#include <QUuid>
#include <QList>
#include "MHelpFuncs.h"


MIhmSessionUserData::MIhmSessionUserData()
{
	m_sInternalSessionID = QUuid::createUuid().toString(QUuid::WithoutBraces);
	//m_sInternalSessionID = m_sInternalSessionID.replace("{","");
	//m_sInternalSessionID = m_sInternalSessionID.replace("}","");
	m_bAuthorized = false;
}

MIhmSessionUserData::~MIhmSessionUserData()
{
	
}


MIhmSessionUserData& MIhmSessionUserData::operator=(const MIhmSessionUserData& right)
{
	m_sInternalSessionID = MHelpFuncs::deepCopy(right.m_sInternalSessionID);
	m_sWebSessionID = MHelpFuncs::deepCopy(right.m_sWebSessionID);
	m_sUserId = MHelpFuncs::deepCopy(right.m_sUserId);
	m_sUserName = MHelpFuncs::deepCopy(right.m_sUserName);
	m_sSecCode = MHelpFuncs::deepCopy(right.m_sSecCode);
	m_bIsSecCodePwd = right.m_bIsSecCodePwd;;
	m_bAuthorized = right.m_bAuthorized;
	m_bIsDesktopUser = right.m_bIsDesktopUser;

	return *this;
}


MIhmSessionUserData * MIhmSessionUserData::findByWebSessionID(QList <MIhmSessionUserData*> *plst, QString sWebSessionId)
{
	MIhmSessionUserData *pRetObj = NULL;

	for (int i = 0; i < plst->size(); ++i) 
	{
		if(plst->at(i)->m_sWebSessionID==sWebSessionId)
		{
			pRetObj = plst->at(i);
			break;
		}
	}

	return pRetObj;	
}

MIhmSessionUserData * MIhmSessionUserData::findByIntSessionID(QList <MIhmSessionUserData*> *plst, QString sInternalSessionId)
{
	MIhmSessionUserData *pRetObj = NULL;

	for (int i = 0; i < plst->size(); ++i) 
	{
		if(plst->at(i)->m_sInternalSessionID==sInternalSessionId)
		{
			pRetObj = plst->at(i);
			break;
		}
	}

	return pRetObj;	
}


bool MIhmSessionUserData::removeIhmSession(QList <MIhmSessionUserData*> *plst, MIhmSessionUserData *pToRemove, bool bDelete)
{
	bool bOK = false;

	if (pToRemove!=NULL)
		for (int i = 0; i < plst->size(); ++i) 
		{
			if(plst->at(i)==pToRemove)
			{
				MIhmSessionUserData *pObj = plst->takeAt(i);
				if(bDelete)
					delete pObj;
				bOK = true;
				break;
			}
		}

	return bOK;	
}
