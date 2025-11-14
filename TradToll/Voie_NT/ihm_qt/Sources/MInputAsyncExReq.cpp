
#include <QStringList>
#include <QDateTime>
#include <QDomDocument>
#include <QTextStream>

#include "MInputAsyncExReq.h"
#include "MHelpFuncs.h"
#include "MIhmConfigGeneral.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}



MInputAsyncExReq::MInputAsyncExReq(MInputAsyncExReq &obj)
{
	*this = obj;
}

MInputAsyncExReq::MInputAsyncExReq()
{

}


MInputAsyncExReq::~MInputAsyncExReq()
{
	while (!m_lstInputFields.isEmpty())
		delete m_lstInputFields.takeFirst();

}


bool MInputAsyncExReq::init(QString sLaneType)
{
	MIhmLaneTypeSettings * pLaneSettings = 
				MIhmConfigGeneral::getCfg()->getLaneTypeSetting(MIhmLaneTypeSettings::enuDESKTOP, sLaneType);

	if(pLaneSettings!=NULL)
	{
		QString sAsyncDef = pLaneSettings->getParam(CFG_LANE_TYPE_ATTRIBUTE_ASYNC_INPUT_FILE);
		
		if(sAsyncDef!="")
			return setDefinition(sAsyncDef);
		else
		{
			//in other cases ... it might be error or it is disabled
			TRACE_W(QString( "MInputAsyncExReq::init:Parameter for the lane type %1 for the lane type %2 is empty! Async input is disabled.")
				.arg(CFG_LANE_TYPE_ATTRIBUTE_ASYNC_INPUT_FILE)
				.arg(sLaneType));
		}
	}

	return false;
}


bool MInputAsyncExReq::verifyResponseFormat(QString sDlgResult)
{
	bool bOK = true;

	TRACE_W("MInputAsyncExReq::verifyResponseFormat: NOT IMPLEMENTED!");

	return bOK;
}


MInputAsyncExReq& MInputAsyncExReq::operator=(const MInputAsyncExReq& right)
{
	this->m_sDescription = right.m_sDescription;
	this->m_sDefinition = right.m_sDefinition;
	
	while (!m_lstInputFields.isEmpty())
		delete m_lstInputFields.takeFirst();

	MInputExField * pCurrent;

	for(int i=0;i<right.m_lstInputFields.size();i++)
	{
		pCurrent = right.m_lstInputFields.at(i);

		if(pCurrent!=NULL)
		{
			MInputExField * pNew = new MInputExField();
			*pNew = *pCurrent;
			m_lstInputFields.append(pNew);
		}
	}

	this->m_bValid = right.m_bValid;

	return *this;	
}


void MInputAsyncExReq::updateInputValues(QString sEncStrValues)
{
	QStringList slstNameValuePairs = sEncStrValues.split(QString(IHM_SAISIE_EX_SEPARATEUR_CHAMP));

	for (int i=1;i<slstNameValuePairs.size()-1;i++)
	{
		QString sNameValuePair = slstNameValuePairs.at(i);

		QString sName = 
			MHelpFuncs::getStringItem(sNameValuePair,0,QString(IHM_SAISIE_EX_SEPARATEUR_PARAM),true);
		QString sValue = 
			MHelpFuncs::getStringItem(sNameValuePair,1,QString(IHM_SAISIE_EX_SEPARATEUR_PARAM),false);


		MInputExField * pField = getInputField(sName);

		if(pField!=NULL)
		{
			pField->setValue(sValue);
		}
		else
		{
			TRACE_W(QString( "MInputAsyncExReq::updateInputValues:Error:Field name:%1 not found !")
									.arg(sName));

		}
	}
}

QString MInputAsyncExReq::getEncStrValues()
{

	QString sRetVal;
	sRetVal += IHM_SAISIE_EX_SEPARATEUR_CHAMP;

	MInputExField * pCurrent;

	for(int i = 0;i<m_lstInputFields.size();i++)
	{
		pCurrent = m_lstInputFields.at(i);
		
		if(pCurrent->hasReturningValue())
		{
			sRetVal += QString("%1~%2").arg(pCurrent->getName()).arg(pCurrent->getValue());
			sRetVal += IHM_SAISIE_EX_SEPARATEUR_CHAMP;
		}

	}

	return sRetVal;
}

