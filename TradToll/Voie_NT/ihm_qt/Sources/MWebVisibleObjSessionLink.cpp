

#include "MIhmWebVisibleObject.h"
#include "MWebVisibleObjSessionLink.h"



MWebVisibleObjSessionLink * MWebVisibleObjSessionLink::findVisibleObjectByName(QVector <MWebVisibleObjSessionLink*> *pVect, QString sName)
{
	MWebVisibleObjSessionLink *pRetObj = NULL;

	for (int i = 0; i < pVect->size(); ++i) 
	{
		if(pVect->at(i)!=NULL)
			if(pVect->at(i)->getName()==sName)
			{
				pRetObj = pVect->at(i);
				break;
			}
	}

	return pRetObj;	
}

//MWebVisibleObjSessionLink * MWebVisibleObjSessionLink::findVisibleObjectByName(QList <MWebVisibleObjSessionLink*> *pList, QString sName)
//{
//	MWebVisibleObjSessionLink *pRetObj = NULL;
//
//	for (int i = 0; i < pList->size(); ++i) 
//	{
//		if(pList->at(i)!=NULL)
//			if(pList->at(i)->getName()==sName)
//			{
//				pRetObj = pList->at(i);
//				break;
//			}
//	}
//
//	return pRetObj;	
//}

bool MWebVisibleObjSessionLink::isReady()
{
	if(m_pVisibleObject!=NULL)
		return m_pVisibleObject->isReady();

	return false;
}

QString MWebVisibleObjSessionLink::getName()
{
	if(m_pVisibleObject!=NULL)
		return m_pVisibleObject->getName();

	return QString();
}

QString MWebVisibleObjSessionLink::getStyle(bool bReadOnly)
{
	if(m_pVisibleObject!=NULL)
		return m_pVisibleObject->getStyle(bReadOnly);

	return QString();
}

QString MWebVisibleObjSessionLink::getTimeStamp()
{
	if(m_pVisibleObject!=NULL)
		return m_pVisibleObject->getTimeStamp();

	return QString();
}

QString MWebVisibleObjSessionLink::getHtml(bool bReadOnly)
{
	if(m_pVisibleObject!=NULL)
		return m_pVisibleObject->getHtml(bReadOnly);

	return QString();
}

MIhmWebVisibleObject::enumWebVisibleObjectType MWebVisibleObjSessionLink::getType()
{
	if(m_pVisibleObject!=NULL)
		return m_pVisibleObject->getType();

	return MIhmWebVisibleObject::enuHtmlUnknownObject;
}


bool MWebVisibleObjSessionLink::isUpdated()
{
	if(m_pVisibleObject!=NULL)
	{
		if(m_pVisibleObject->getType() == MIhmWebVisibleObject::enuHtmlOpenLink)
		{
			return m_pVisibleObject->isUpdated();
		}
		else if(m_pVisibleObject->getType() == MIhmWebVisibleObject::enuHtmlInControlMode)
		{
			return true; 
		}
		else if(m_pVisibleObject->getTimeStamp()==m_sAckTimeStamp) 
			return false;
	}

	return true;
}

bool MWebVisibleObjSessionLink::isUsedAtMiniWeb()
{
	if(m_pVisibleObject!=NULL)
		return m_pVisibleObject->isUsedAtMiniWeb();

	else
		return false;
}


void MWebVisibleObjSessionLink::setBeingUpdated()
{
	if(m_pVisibleObject!=NULL)
		m_sAckTimeStamp = m_pVisibleObject->getTimeStamp();
}

bool MWebVisibleObjSessionLink::getActionForCtrlId(QString sCtrlId, QString &sActionID, QString &sParams)
{
	if(m_pVisibleObject!=NULL)
		return m_pVisibleObject->getActionForCtrlId(sCtrlId, sActionID, sParams);	
	else
		return false;
}


void MWebVisibleObjSessionLink::setAckTimeStamp(QString sTS)
{
	if(m_pVisibleObject!=NULL)
	{
		if(m_pVisibleObject->getId() > MIhmWebVisibleObject::enuWebVisibleObjectLastCommon)
		{
			m_pVisibleObject->setAckTimeStamp(sTS);
		}
	}

	m_sAckTimeStamp = sTS;
}
 	


void MWebVisibleObjSessionLink::forceUpdateToEveryone()
{ 
	if(m_pVisibleObject!=NULL)
	{
		m_pVisibleObject->forceUpdate();
	}

	m_sAckTimeStamp = "";
}

void MWebVisibleObjSessionLink::forceUpdateForSession()
{ 
	m_sAckTimeStamp = "";
}	