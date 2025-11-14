
#include "MIhmAniVirtObjects.h"
#include "MStringDetection.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}

 
MIhmAniVirtObjects::MIhmAniVirtObjects()
{
// 	for (int i = 0; i < m_vectVirtObjects.size(); ++i) 
// 	{
// 		if(m_vectVirtObjects.at(i)!=NULL)
// 			m_vectVirtObjects.at(i)->setInitialized();
// 	}

}


MIhmAniVirtObjects::~MIhmAniVirtObjects()
{

}




bool MIhmAniVirtObjects::initialize(QString sModuleConfigKey)
{
	bool bRetVal = false;
	

	bRetVal = loadObjectsDefaults(sModuleConfigKey);


	return bRetVal;
}

bool MIhmAniVirtObjects::loadObjectsDefaults(QString sModuleConfigKey)
{
	bool bRetVal = false;

	//load detection pairs from registry ..
	MIhmVirtStrDetectConfig * pVirtObj = (MIhmVirtStrDetectConfig *)getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtStrDetectConfigID);
	bRetVal = MStringDetection::loadDetectionPairs(sModuleConfigKey,&(pVirtObj->m_lstActivePairs));


	return bRetVal; 
}

