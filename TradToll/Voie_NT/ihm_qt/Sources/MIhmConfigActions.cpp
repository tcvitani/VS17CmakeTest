

#include <QString>
#include <QDomDocument>
#include <QFile>

#include "MIhmConfigActions.h"
#include "MIhmAction.h"
#include "MHelpFuncs.h"
#include "MTracer.h"



#define XML_ELEMENT_IHM_CONFIG_ROOT			"IHM_CONFIG_ACTIONS"
#define XML_ELEMENT_IHM_CONFIG_ACTIONS			"ACTIONS"
#define XML_ELEMENT_IHM_CONFIG_ACTION			"ACTION"

#define XML_ACTION_ATTRIBUTE_ID			"ID"
#define XML_ACTION_ATTRIBUTE_TYPE		"TYPE"
#define XML_ACTION_ATTRIBUTE_PARAM		"PARAM"
#define XML_ACTION_ATTRIBUTE_CURSOR_ID	"CURSOR_ID"



MIhmConfigActions::MIhmConfigActions()
{

}


MIhmConfigActions::~MIhmConfigActions()
{
	while(!m_lstAvailableActions.isEmpty())
		delete m_lstAvailableActions.takeLast();
}

bool MIhmConfigActions::lessThan(const MIhmAction* x1, const MIhmAction* x2) 
{ 
	return *x1 < *x2; 

}

bool MIhmConfigActions::loadConfigFromFile(QString sConfigFilePath)
{
	bool bRetValue = false;
    TRACE_D(QString( "MIhmConfigActions::loadConfigFromFile from: %1").
								arg(sConfigFilePath));


	//empty current action list
	while(!m_lstAvailableActions.isEmpty())
		delete m_lstAvailableActions.takeFirst();

	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument xmlData;

	QFile file(sConfigFilePath);
	
	if(file.open(QFile::ReadOnly))
	{

		bool bRet = xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn);

		if(bRet)
		{
			QDomElement root = xmlData.documentElement();
			if(root.tagName() != XML_ELEMENT_IHM_CONFIG_ROOT)
			{
				TRACE_W("MIhmConfigActions::loadConfigFromFile: Invalid root element!");
				bRet = false;
			}
			else
			{
				QDomNodeList lstActionItems = root.elementsByTagName(XML_ELEMENT_IHM_CONFIG_ACTIONS);
				
				if(lstActionItems.count()>0)
				{
					QDomNode nodesActions = lstActionItems.at(0);
					//root element for all lane types
					QDomElement elActions = nodesActions.toElement();

					QDomNodeList lstAction = elActions.elementsByTagName(XML_ELEMENT_IHM_CONFIG_ACTION);
					
					QDomNode currAction; 
					
					for(int i=0;i<lstAction.count();i++)
					{
						currAction = lstAction.at(i); 

						QString sId = MHelpFuncs::getAttributeText(&currAction, XML_ACTION_ATTRIBUTE_ID);
						QString sType = MHelpFuncs::getAttributeText(&currAction, XML_ACTION_ATTRIBUTE_TYPE);
						QString sParamValue = MHelpFuncs::getAttributeText(&currAction, XML_ACTION_ATTRIBUTE_PARAM);
						QString sCursorID = MHelpFuncs::getAttributeText(&currAction, XML_ACTION_ATTRIBUTE_CURSOR_ID);

						MIhmAction * pNewAction = MIhmAction::createAction(sId, sType,sParamValue, sCursorID);
							
						if(pNewAction!=NULL)
						{
							m_lstAvailableActions.append(pNewAction);
							bRetValue = true; //at least one should be added;
						}
					}
				}
			}
		}
		else
		{
			QString sMsg = QString("MIhmConfigActions::loadConfigFromFile: Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
			TRACE_W(sMsg);
		}
	}
	else
	{
		TRACE_W(QString("MIhmConfigActions::loadConfigFromFile: Unable to open file %1").
								arg(sConfigFilePath));
	}


	//qSort(m_lstAvailableActions.begin(), m_lstAvailableActions.end(), MIhmConfigActions::lessThan);
	std::sort(m_lstAvailableActions.begin(), m_lstAvailableActions.end(), MIhmConfigActions::lessThan);

	return bRetValue;
}






MIhmAction * MIhmConfigActions::findAction(QString sActionID)
{
	MIhmAction *pCurrent = NULL;
	MIhmAction oWanted;

	oWanted.setActionID(sActionID);
	
	QList<MIhmAction *>::iterator it;
	
	//it = qBinaryFind(m_lstAvailableActions.begin(), m_lstAvailableActions.end(), &oWanted, MIhmConfigActions::lessThan);
	it = std::lower_bound(m_lstAvailableActions.begin(), m_lstAvailableActions.end(), &oWanted, MIhmConfigActions::lessThan);
	

	if( it !=  m_lstAvailableActions.end())
	{
		pCurrent = *it;
		return pCurrent;
	}

	return NULL; //NULL
}


QString MIhmConfigActions::getCursorIdForAction(QString sActionID)
{
	MIhmAction *pCurrent = findAction(sActionID);


	if(pCurrent!=NULL)
		return pCurrent->getCursorID();

	return QString("");
}


