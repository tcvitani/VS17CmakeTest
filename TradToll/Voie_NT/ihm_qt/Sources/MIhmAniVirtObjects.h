#ifndef MIHM_ANI_VIRT_OBJECTS_H
#define MIHM_ANI_VIRT_OBJECTS_H


#include "MIhmVirtObjectsModel.h"

#include <QList.h>

class MIhmAniVirtObjects: public MIhmVirtObjectsModel 
{
public:
	MIhmAniVirtObjects();
	~MIhmAniVirtObjects();

	bool initialize(QString sModuleConfigKey);

private:
	bool loadObjectsDefaults(QString sModuleConfigKey);

};




#endif


