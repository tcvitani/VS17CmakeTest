#include "MIhmConfigDynamic.h"
#include "MHelpFuncs.h"
#include "MTracer.h"

#include <QDomDocument>
#include <QFile>

extern "C" {
	#include <ihm.h>
	#include "ihm_loc.h"
}

MIhmConfigDynamic::MIhmConfigDynamic()
{
	m_iMaxWebClients = 3;
	m_iMaxMiniWebClients = 2;
	m_iIdleTimeoutDsk = 30; //DEFAULT
	m_iTakeCtrlTimeout = 5000; //DEFAULT
}


MIhmConfigDynamic::~MIhmConfigDynamic()
{
	
}

bool MIhmConfigDynamic::loadConfigFromFile(QString sConfigFilenameAndPath)
{
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument xmlData;
	
	QFile file(sConfigFilenameAndPath);
	
	if(file.open(QFile::ReadOnly))
	{
		
		if(xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn))
		{
			QDomElement root = xmlData.documentElement();
			
			if(QString::compare(root.tagName(), "CONFIG", Qt::CaseInsensitive)!=0)
			{
				TRACE_W("MIhmConfigDynamic::loadConfigFromFile: Invalid root element!");
				return false;
			}
		}
		else
		{
			QString sMsg = QString("MIhmConfigDynamic::loadConfigFromFile:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
			TRACE_W(sMsg);
			return false;
		}
	}
	else
	{
		TRACE_W(QString("MIhmConfigDynamic::loadConfigFromFile: Unable to open file %1").arg(sConfigFilenameAndPath) );
		return false;
	}
	
	QDomElement root = xmlData.documentElement();
	QDomNodeList lstItems = root.childNodes();
	QDomNode currItem;
	
	for(int i=0;i<lstItems.count();i++)
	{
		currItem = lstItems.at(i); 
		
		if(QString::compare(currItem.nodeName(), "PARAM",Qt::CaseInsensitive)==0)
		{
			// check param attribute name and get value 
			QString sParamID = MHelpFuncs::getAttributeText(&currItem, "ID");
			QString sParamValue = MHelpFuncs::getAttributeText(&currItem, "VALUE");

			if(QString::compare(sParamID, IHM_DYNAMIC_CONF_PARAM_MAX_WEB_CLIENTS,Qt::CaseInsensitive)==0)
			{
				if(!getIntValue(sParamValue, sParamID, m_iMaxWebClients))
					m_iMaxWebClients = 3; //DEFAULT 
			}
			if(QString::compare(sParamID, IHM_DYNAMIC_CONF_PARAM_MAX_MINIWEB_CLIENTS,Qt::CaseInsensitive)==0)
			{
				if(!getIntValue(sParamValue, sParamID, m_iMaxMiniWebClients))
					m_iMaxMiniWebClients = 2; //DEFAULT 
			}
			else if(QString::compare(sParamID, IHM_DYNAMIC_CONF_PARAM_IDLE_TIMEOUT_DSK,Qt::CaseInsensitive)==0)
			{
				if(!getIntValue(sParamValue, sParamID, m_iIdleTimeoutDsk))
					m_iIdleTimeoutDsk = 30; //DEFAULT

			}
			else if(QString::compare(sParamID, IHM_DYNAMIC_CONF_PARAM_IDLE_TIMEOUT_WEB,Qt::CaseInsensitive)==0)
			{
				if(!getIntValue(sParamValue, sParamID, m_iIdleTimeoutWeb))
					m_iIdleTimeoutWeb = 30; //DEFAULT

			}
			else if(QString::compare(sParamID, IHM_DYNAMIC_CONF_PARAM_TAKE_CTRL_TIMEOUT,Qt::CaseInsensitive)==0)
			{
				if(!getIntValue(sParamValue, sParamID, m_iTakeCtrlTimeout))
					m_iTakeCtrlTimeout = 5000; //DEFAULT

			}
		}

	}

	
	return true;
}



bool MIhmConfigDynamic::getIntValue(QString sParamValue, QString sParamName, int &iRetVal)
{
	bool bOK = false;

	iRetVal = sParamValue.toInt(&bOK, 10);
	
	if(!bOK)
	{
		TRACE_W(QString("MIhmConfigDynamic::getIntValue: Error param %1[%2]").arg(sParamName).arg(sParamValue));
	}

	return bOK;
}


