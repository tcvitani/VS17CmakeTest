

#include "MKybAniInitData.h"
#include "MTracer.h"
#include <QDir>

extern "C" {
	#include <virt_kyb.h>
};


MKybAniInitData::MKybAniInitData()
{
	
}


MKybAniInitData::~MKybAniInitData()
{

}

QString MKybAniInitData::getMboxName()
{
	return m_sMboxName;
}


bool MKybAniInitData::initialize(QString szMboxName, QString sConfigKey)
{
	m_sMboxName = szMboxName;
	m_sConfigKey = sConfigKey;

	m_iPoolId = 0;
	
	return INST_INIT_OK;
}
