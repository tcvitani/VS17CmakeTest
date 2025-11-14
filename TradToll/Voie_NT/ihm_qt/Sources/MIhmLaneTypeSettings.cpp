

#include "MIhmLaneTypeSettings.h"
#include "MHelpFuncs.h"
#include "MTracer.h"

#include <QFile>
#include <QDir>


LaneTypeVisObjParams::LaneTypeVisObjParams()
{

}

LaneTypeVisObjParams::~LaneTypeVisObjParams()
{
	qDeleteAll(m_lstParams);
	qDeleteAll(m_lstSubTemplates);
}


QString LaneTypeVisObjParams::getParam(QString sParamName)
{
	QString sRetVal;

	for (int i=0;i<m_lstParams.size();++i)
	{
		if(m_lstParams.at(i)->param_name==sParamName)
		{
			sRetVal = m_lstParams.at(i)->param_value;
			break;
		}
	}

	return sRetVal;
}


QString LaneTypeVisObjParams::getSubTemplateContent(QString sParamName)
{
	QString sRetVal;

	for (int i=0;i<m_lstSubTemplates.size();++i)
	{
		if(m_lstSubTemplates.at(i)->param_name==sParamName)
		{
			sRetVal = m_lstSubTemplates.at(i)->param_value;
			break;
		}
	}

	return sRetVal;
}


//----------------------------------------------------------------------------

MIhmLaneTypeSettings::MIhmLaneTypeSettings(enumSettingsType eType, QString sLaneType)
{
	m_sLaneType = sLaneType;
	m_eType = eType;
}


MIhmLaneTypeSettings::~MIhmLaneTypeSettings()
{
	 while (!m_lstLaneTypeParams.isEmpty())
		 delete m_lstLaneTypeParams.takeFirst();
	 
	 while (!m_lstLaneTypeVisObjParams.isEmpty())
		 delete m_lstLaneTypeVisObjParams.takeFirst();

}


// bool MIhmLaneTypeSettings::loadSettings(QString sLaneTypesRegKey)
// {
//     DWORD dwIndex = 0;
// 	char pcNomValeur[MAX_PATH];
// 	DWORD dwTailleNom = MAX_PATH;
// 	char pcValeur[MAX_PATH];
// 	DWORD dwTailleValeur = MAX_PATH;
// 	char pcRegKey[MAX_PATH];
// 
// 	m_sSourceRegKey = sLaneTypesRegKey;
// 
// 	strcpy(pcRegKey, m_sSourceRegKey.toLatin1().data());
// 
// 	DWORD dwRetVal = REG_Enum_Valeurs_Chaine(
//                       CSR_REG_KEYi_ROOT,
//                       pcRegKey,
//                       dwIndex,
//                       pcNomValeur,
//                       &dwTailleNom,
//                       pcValeur,
//                       &dwTailleValeur);   
//     
// 
//     
//     while(dwRetVal == ERROR_SUCCESS)
//     {
// 			
// 		
//         if (dwTailleNom!=0 && dwTailleValeur!=0)
//         {
// 			LaneTypeParam * pNewParam;
// 			pNewParam = new LaneTypeParam();
// 
// 			pNewParam->param_name = QString(pcNomValeur);
// 			pNewParam->param_value = QString(pcValeur);
// 
// 			m_lstLaneTypeParams.append(pNewParam);
//         }
//          
//         // Enumerer la valeur suivante
//         dwIndex = dwIndex + 1;
// 		dwTailleNom = MAX_PATH;
// 		dwTailleValeur = MAX_PATH;
// 
//         dwRetVal = REG_Enum_Valeurs_Chaine(CSR_REG_KEYi_ROOT,
//                       pcRegKey,
//                       dwIndex,
//                       pcNomValeur,
//                       &dwTailleNom,
//                       pcValeur,
//                       &dwTailleValeur);
//     }
//     
// 	loadHtmlTemplates(); 
// 	
// 	return true;
// }


LaneTypeParam * MIhmLaneTypeSettings::newParam(QString sParamName, QString sValue)
{
	LaneTypeParam * pParam;

	pParam = new LaneTypeParam();
	pParam->param_name = sParamName;
	pParam->param_value = sValue;

	return pParam;

}


bool MIhmLaneTypeSettings::verifyIfAllDskParametersExist()
{
	bool bRetVal = true;

	QStringList sLst;
	sLst << CFG_LANE_TYPE_PARAM_MAIN_DIALOG
		<< CFG_LANE_TYPE_PARAM_TSKBAR_BROWSER_DLG_TEMPLATE
		<< CFG_LANE_TYPE_PARAM_TSKBAR_DLG_TYPES 
		<< CFG_LANE_TYPE_PARAM_CLIENT_CSS_FILE 
		<< CFG_LANE_TYPE_PARAM_LOGIN_DLG_TEMPLATE
		<< CFG_LANE_TYPE_PARAM_CLIENT_DLG_EMBEDED_TO 
		<< CFG_LANE_TYPE_PARAM_TAKE_OVER_DLG_TEMPLATE;


	QString sCurrent;
	QString sValue;

	for (int i=0; i<sLst.size();i++)
	{
		sCurrent = sLst.at(i);
		sValue = getParam(sCurrent);

		if (sValue == "")
		{
			TRACE_W(QString( "MIhmLaneTypeSettings::verifyIfAllDskParametersExist: Parameter [%1] is empty for the lane type [%2]. Is this intent?")
									.arg(sCurrent)
									.arg(m_sLaneType));
			bRetVal = false;
		}

	}

	return bRetVal;
}


bool MIhmLaneTypeSettings::verifyIfAllWebParametersExist()
{
	bool bRetVal = true;

	QStringList sLst;
	sLst << CFG_LANE_TYPE_PARAM_WEB_MAIN_PAGE
		<< CFG_LANE_TYPE_PARAM_WEB_HTMLTemplateRoot;
	

	QString sCurrent;
	QString sValue;

	for (int i=0; i<sLst.size();i++)
	{
		sCurrent = sLst.at(i);
		sValue = getParam(sCurrent);

		if (sValue == "")
		{
			TRACE_W(QString( "MIhmLaneTypeSettings::verifyIfAllWebParametersExist: Parameter [%1] is empty for the lane type [%2]. Is this intent?")
									.arg(sCurrent)
									.arg(m_sLaneType));
			bRetVal = false;
		}

	}

	return bRetVal;
}



bool MIhmLaneTypeSettings::loadXMLSettings(QDomNode nodeLaneType)
{
	
	QString sAsyncFileName = MHelpFuncs::getAttributeText(&nodeLaneType, CFG_LANE_TYPE_ATTRIBUTE_ASYNC_INPUT_FILE);
	
	m_lstLaneTypeParams.append(newParam(CFG_LANE_TYPE_ATTRIBUTE_ASYNC_INPUT_FILE,sAsyncFileName));

	if(m_eType==enuDESKTOP)
	{
		//select DESKTOP element
		QDomElement elemDesktop = nodeLaneType.firstChildElement(CFG_LANE_TYPE_ELEMENT_DESKTOP);

		if(!elemDesktop.isNull())
		{
			//select PARAM elements
			QDomNodeList lstNodesParams = elemDesktop.elementsByTagName(CFG_LANE_TYPE_ELEMENT_PARAM);

			QDomNode nodeCurrParam; 
			
			for(int i=0;i<lstNodesParams.count();i++)
			{
				//extract id and values
				nodeCurrParam = lstNodesParams.at(i); 
				
				QString sParamID = MHelpFuncs::getAttributeText(&nodeCurrParam, CFG_VISIBLE_OBJ_ATTRIBUTE_ID);
				QString sParamValue = MHelpFuncs::getAttributeText(&nodeCurrParam, CFG_VISIBLE_OBJ_ATTRIBUTE_VALUE);
				
				// add each to the m_lstLaneTypeParams list
				m_lstLaneTypeParams.append(newParam(sParamID,sParamValue));

			}

			//just to trace if something missing 
			verifyIfAllDskParametersExist();
				
			//select VISIBLE_OBJ elements
			QDomNodeList lstNodesVisObj = elemDesktop.elementsByTagName(CFG_VISIBLE_OBJ_ELEMENT);

			QDomNode nodeCurrVisObj; 
			QString sObjID,sObjVisible,sObjActionId,sObjTemplate,sObjMaxRows,sObjSetFocusKey;
			QString sObjIsIconMode, sObjNotUsed, sObjLeft, sObjTop, sObjWidth, sObjHeight, sObjShowText, sObjShowIcon, sObjTooltipShowLastIcon;
			
			for(int i=0;i<lstNodesVisObj.count();i++)
			{
				//extract id and values
				nodeCurrVisObj = lstNodesVisObj.at(i); 
				
				sObjID = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_ID);
				sObjVisible = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_VISIBLE);
				sObjNotUsed = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_NOT_USED);
				sObjLeft = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_LEFT);
				sObjTop = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_TOP);
				sObjWidth = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_WIDTH);
				sObjHeight = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_HEIGHT);
				sObjActionId = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_ACTION_ID);
				sObjTemplate = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_TEMPLATE);
				sObjMaxRows =  MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_MAX_ROWS); 
				sObjSetFocusKey = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_SET_FOCUS_KEY);
				sObjIsIconMode = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_IS_ICON_MODE);
				sObjShowText = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_SHOW_TEXT);
				sObjShowIcon = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_SHOW_ICON);
				sObjTooltipShowLastIcon = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_TOOLTIP_SHOW_LAST_ITEM);
				

				LaneTypeVisObjParams * pVisObj = new LaneTypeVisObjParams();
				
				pVisObj->m_sVisObjID = sObjID;
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_VISIBLE,sObjVisible));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_NOT_USED,sObjNotUsed));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_LEFT,sObjLeft));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_TOP,sObjTop));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_WIDTH,sObjWidth));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_HEIGHT,sObjHeight));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_ACTION_ID,sObjActionId));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_TEMPLATE,sObjTemplate));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_SET_FOCUS_KEY,sObjSetFocusKey));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_IS_ICON_MODE,sObjIsIconMode));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_MAX_ROWS,sObjMaxRows));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_SHOW_TEXT,sObjShowText));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_SHOW_ICON,sObjShowIcon));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_TOOLTIP_SHOW_LAST_ITEM, sObjTooltipShowLastIcon));

				// add each to the m_lstLaneTypeVisObjParams list
				m_lstLaneTypeVisObjParams.append(pVisObj);
			}

		}
		else
		{
			TRACE_W(QString( "MIhmLaneTypeSettings::loadXMLSettings: Unable to select element [%1] for lane type [%2]!")
									.arg(CFG_LANE_TYPE_ELEMENT_DESKTOP)
									.arg(m_sLaneType));

		}
		
	}
	else if(m_eType==enuWEB)
	{

		//select WEB element
		QDomElement elemWEB = nodeLaneType.firstChildElement(CFG_LANE_TYPE_ELEMENT_WEB);

		if(!elemWEB.isNull())
		{
			//select PARAM elements
			QDomNodeList lstNodesParams = elemWEB.elementsByTagName(CFG_LANE_TYPE_ELEMENT_PARAM);

			QDomNode nodeCurrParam; 
			QString sParamID;
			QString sParamValue;

			for(int i=0;i<lstNodesParams.count();i++)
			{
				//extract id and values
				nodeCurrParam = lstNodesParams.at(i); 
				
				sParamID = MHelpFuncs::getAttributeText(&nodeCurrParam, CFG_VISIBLE_OBJ_ATTRIBUTE_ID);
				sParamValue = MHelpFuncs::getAttributeText(&nodeCurrParam, CFG_VISIBLE_OBJ_ATTRIBUTE_VALUE);
				
				// add each to the m_lstLaneTypeParams list
				m_lstLaneTypeParams.append(newParam(sParamID,sParamValue));

			}

			
			//just to trace if something missing 
			verifyIfAllWebParametersExist();

			
			//select VISIBLE_OBJ elements
			QDomNodeList lstNodesVisObj = elemWEB.elementsByTagName(CFG_VISIBLE_OBJ_ELEMENT);

			QDomNode nodeCurrVisObj; 
			QString sObjID;
			QString sObjActionId;
			QString sObjTemplate;
			QString sObjVisible;
			QString sObjNotUsed;
			QString sObjFormatFile; 
			QString sObjMaxRows; 
			QString sObjTemplateContent;
			QString sObjUsedAtMiniWeb;

			for(int i=0;i<lstNodesVisObj.count();i++)
			{
				//extract id and values
				nodeCurrVisObj = lstNodesVisObj.at(i); 
				
				sObjID = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_ID);
				sObjActionId = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_ACTION_ID);
				sObjTemplate = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_TEMPLATE);
				sObjVisible = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_VISIBLE);
				sObjNotUsed = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_NOT_USED);
				sObjFormatFile = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_FORMAT_FILE); 
				sObjMaxRows = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_MAX_ROWS); 
				sObjUsedAtMiniWeb = MHelpFuncs::getAttributeText(&nodeCurrVisObj, CFG_VISIBLE_OBJ_ATTRIBUTE_USED_AT_MINI_WEB); 
				
				LaneTypeVisObjParams * pVisObj = new LaneTypeVisObjParams();
				
				pVisObj->m_sVisObjID = sObjID;
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_ACTION_ID,sObjActionId));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_VISIBLE,sObjVisible));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_NOT_USED,sObjNotUsed));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_FORMAT_FILE,sObjFormatFile));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_MAX_ROWS,sObjMaxRows));
				pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_USED_AT_MINI_WEB,sObjUsedAtMiniWeb));
				
				QString sTemplatesRoot = getParam(CFG_LANE_TYPE_PARAM_WEB_HTMLTemplateRoot);
				QString sPath;

				if(sObjTemplate!="")
				{
					sPath = QDir::cleanPath(sTemplatesRoot + "/" + sObjTemplate);
					
					sObjTemplateContent = MHelpFuncs::loadFileContent(sPath);
				
				
					if(sObjTemplateContent == "")
					{
						TRACE_W(QString( "MIhmLaneTypeSettings::loadXMLSettings: Invalid web template file [%1] for object [%2]!")
									.arg(sPath)
									.arg(sObjID));
					}
					else
						pVisObj->m_lstParams.append(newParam(CFG_VISIBLE_OBJ_ATTRIBUTE_TEMPLATE,sObjTemplateContent));
				}
				else
				{
					QString sMsg;
					sMsg = QString("MIhmLaneTypeSettings::loadXMLSettings:*** No web template defined for object [%1]!").arg(sObjID);
					TRACE_D(sMsg);
				}

				//Add each visible object parameters structure to the m_lstLaneTypeVisObjParams list
				m_lstLaneTypeVisObjParams.append(pVisObj);

				QDomElement elemCurVisObj = nodeCurrVisObj.toElement();

				//Load specific PARAM objects for the visible object
				QDomNodeList lstNodesSpecParams = elemCurVisObj.elementsByTagName(CFG_VISIBLE_OBJ_ELEMENT_PARAM);
				
				QDomNode nodeCurrSpecParam; 
				
				for(int i=0;i<lstNodesSpecParams.count();i++)
				{
					nodeCurrSpecParam = lstNodesSpecParams.at(i);
					sParamID = MHelpFuncs::getAttributeText(&nodeCurrSpecParam, CFG_VISIBLE_OBJ_ATTRIBUTE_ID);
					sParamValue = MHelpFuncs::getAttributeText(&nodeCurrSpecParam, CFG_VISIBLE_OBJ_ATTRIBUTE_VALUE);

					pVisObj->m_lstParams.append(newParam(sParamID,sParamValue));
				}

				//Load sub templates for the visible object
				QDomNodeList lstNodesSubTemplate = elemCurVisObj.elementsByTagName(CFG_VISIBLE_OBJ_ELEMENT_SUB_TEMPLATE);
				
				QDomNode nodeCurrSubTemplate; 
				QString sSubObjID;
				QString sSubObjValue;
				QString sSubObjTemplateContent;

				for(int i=0;i<lstNodesSubTemplate.count();i++)
				{
					nodeCurrSubTemplate = lstNodesSubTemplate.at(i);
					sSubObjID = MHelpFuncs::getAttributeText(&nodeCurrSubTemplate, CFG_VISIBLE_OBJ_ATTRIBUTE_ID);
					sSubObjValue = MHelpFuncs::getAttributeText(&nodeCurrSubTemplate, CFG_VISIBLE_OBJ_ATTRIBUTE_VALUE);

					sPath = QDir::cleanPath(sTemplatesRoot + "/" + sSubObjValue);

					sSubObjTemplateContent = MHelpFuncs::loadFileContent(sPath);

					if(sSubObjTemplateContent == ""|| sSubObjValue =="")
					{
						TRACE_W(QString( "MIhmLaneTypeSettings::loadXMLSettings: Invalid template file [%1] for object [%2]!")
									.arg(sPath)
									.arg(sObjID));
					}
					else
						pVisObj->m_lstSubTemplates.append(newParam(sSubObjID,sSubObjTemplateContent));
				
				}
					
			}
		}
		else
		{
			TRACE_W(QString( "MIhmLaneTypeSettings::loadXMLSettings: Unable to select element [%1] for lane type [%2]!")
									.arg(CFG_LANE_TYPE_ELEMENT_WEB)
									.arg(m_sLaneType));

		}

			
	}

	return false;
}


LaneTypeVisObjParams * MIhmLaneTypeSettings::getVisObjParams(QString sObjID)
{
	LaneTypeVisObjParams *pVisObjParam=NULL;
	LaneTypeVisObjParams *pVisObjCurrent;

	for (int i=0;i<m_lstLaneTypeVisObjParams.size();++i)
	{
		pVisObjCurrent = m_lstLaneTypeVisObjParams.at(i);

		if(pVisObjCurrent->m_sVisObjID == sObjID)
		{
			pVisObjParam = pVisObjCurrent;
			break;
		}
	}
	
	return pVisObjParam;
}




QString MIhmLaneTypeSettings::getParam(QString sParamName)
{
	QString sRetVal;

	for (int i=0;i<m_lstLaneTypeParams.size();++i)
	{
		if(m_lstLaneTypeParams.at(i)->param_name==sParamName)
		{
			sRetVal = m_lstLaneTypeParams.at(i)->param_value;
			break;
		}
	}

	return sRetVal;
}



QString MIhmLaneTypeSettings::getVisObjParamValue(QString sObjID, QString sParamName)
{
	QString sRetVal;

	LaneTypeVisObjParams * pVisObjParams = getVisObjParams(sObjID);
	
	if(pVisObjParams!=NULL)
		sRetVal = pVisObjParams->getParam(sParamName);

	return sRetVal;
}


QString MIhmLaneTypeSettings::getVisObjHtmlTemplate(QString sObjID)
{
	QString sRetVal;

	LaneTypeVisObjParams * pVisObjParams = getVisObjParams(sObjID);
	
	if(pVisObjParams!=NULL)
		sRetVal = pVisObjParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_TEMPLATE) ;
	
	return sRetVal;
}


QString MIhmLaneTypeSettings::getVisObjHtmlSubTemplate(QString sObjID, QString sParamName)
{
	QString sRetVal;

	LaneTypeVisObjParams * pVisObjParams = getVisObjParams(sObjID);
	
	if(pVisObjParams!=NULL)
		sRetVal = pVisObjParams->getSubTemplateContent(sParamName) ;

	return sRetVal;
}


