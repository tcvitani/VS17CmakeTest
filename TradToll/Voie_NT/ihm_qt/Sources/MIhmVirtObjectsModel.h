#ifndef MIHM_VIRT_OBJECTS_MODEL_H
#define MIHM_VIRT_OBJECTS_MODEL_H


#include "MIhmVirtualObject.h"

#include <QVector.h>

class MIhmVirtObjectsModel
{
public:
	MIhmVirtObjectsModel();
	~MIhmVirtObjectsModel();

	MIhmVirtualObject * getVirtualObjectByID(MIhmVirtualObject::enumVirtualObjectId eID);
	
	bool getIfObjectsChanged();
	int getNumUpdatedObjects();

	bool copyUpdatedVirtObjects(MIhmVirtObjectsModel * pOtherModel, bool bResetUpdated);

	QVector <MIhmVirtualObject*> * getVirtObjVector(){return &m_vectVirtObjects;};
protected:
	QVector <MIhmVirtualObject*> m_vectVirtObjects;
};




#endif


