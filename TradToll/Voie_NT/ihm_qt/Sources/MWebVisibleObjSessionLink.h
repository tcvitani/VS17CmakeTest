#ifndef MIHM_WEB_VISIBLE_OBJ_SESSION_DATA_H
#define MIHM_WEB_VISIBLE_OBJ_SESSION_DATA_H


#include <QString>

#include "MIhmWebVisibleObject.h"


class MWebVisibleObjSessionLink 
{
public:
	MIhmWebVisibleObject* m_pVisibleObject;
	QString	m_sAckTimeStamp; //timestamp that the web client confirmed reception 

	bool isReady();
	QString getName();
	QString getStyle(bool bReadOnly);
	QString getTimeStamp();
	QString getHtml(bool bReadOnly);
	bool getActionForCtrlId(QString sCtrlId, QString &sActionID, QString &sParams);	
	
	MIhmWebVisibleObject::enumWebVisibleObjectType getType();

	bool isUpdated();
	void setBeingUpdated();
	void setAckTimeStamp(QString sTS);
	void forceUpdateToEveryone();
	void forceUpdateForSession();
	bool isUsedAtMiniWeb();

	static MWebVisibleObjSessionLink * findVisibleObjectByName(QVector <MWebVisibleObjSessionLink*> *pVect, QString sName);
//	static MWebVisibleObjSessionLink * findVisibleObjectByName(QList <MWebVisibleObjSessionLink*> *pList, QString sName);

};



#endif


