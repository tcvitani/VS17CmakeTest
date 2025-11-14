
#include <QStringList>
#include <QDateTime>
#include <QDomDocument>
#include <QTextStream>

#include "MInputExReq.h"
#include "MHelpFuncs.h"
#include "MIhmConfigGeneral.h"
#include "MTracer.h"


extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}



MInputExReq::MInputExReq(MInputExReq &obj)
{
	*this = obj;
}

MInputExReq::MInputExReq()
		:m_bValid(false)	
{

}

MInputExReq& MInputExReq::operator=(const MInputExReq& right)
{

	
	this->m_sDescription = MHelpFuncs::deepCopy(right.m_sDescription);
	this->m_sDefinition = MHelpFuncs::deepCopy(right.m_sDefinition);

	
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

MInputExReq::~MInputExReq()
{
	while (!m_lstInputFields.isEmpty())
		delete m_lstInputFields.takeFirst();

}


bool MInputExReq::setDefinition(QString sDefinitionFile)
{
	bool bRetVal = false;

	while (!m_lstInputFields.isEmpty())
		delete m_lstInputFields.takeFirst();


	QString sFullPath = MIhmConfigGeneral::getCfg()->getUIInputExDefPath(sDefinitionFile);
	QString sXml = MHelpFuncs::loadFileContent(sFullPath);

	QDomDocument m_xmlData;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool bRet = m_xmlData.setContent(sXml, &errorStr, &errorLine, &errorColumn);

	if(bRet)
	{
		QDomElement root = m_xmlData.documentElement();
		if(root.tagName() != IHM_SAISIE_EX_ROOT_DEF)
		{
			TRACE_W( "MInputExReq::setDefinition:Invalid root element!");
			return false;
		}

		m_sDskCSSFile = MHelpFuncs::getAttributeText(&root, IHM_SAISIE_EX_ATT_DSKCSS_FILE);
		
		QDomNodeList lstItems = root.childNodes();
		QDomNode currItem;
		QString sName, sType, sFormat, sFormatDetail,sMinLen,sMaxLen, sReadOnly, sMultiselection,sValidator,sBtnId;
		
		for(int i=0;i<lstItems.count();i++)
		{
			currItem = lstItems.at(i); 
			if(currItem.nodeName()==IHM_SAISIE_EX_ELEMENT)
			{
				sType = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_TYPE, true);
				sName = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_NAME, true);
				sFormat = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_FORMAT);	
				sFormatDetail = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_FORMAT_DETAIL);	
				sReadOnly = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_READONLY);	
				sMinLen = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_MIN_LEN);	
				sMaxLen = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_MAX_LEN);	
				sMultiselection = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_MULTISELECTION);	
				sValidator = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_VALIDATOR);	
				sBtnId = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_BTN_ID);	

				//create the input item and add it to the definition list
				MInputExField * pNew = createInputField(sType,sName,sFormat,sFormatDetail,sMinLen,sMaxLen,sReadOnly, sMultiselection, sValidator, sBtnId);
				if(pNew->getType() == MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT)
				{
					if(pNew->getFormatType() == MInputExField::enuFIELD_FORMAT_CALCULATION)
					{
						QString sResultPrec, sEnableLabelSwitching, sThreshold, sLabelFalse, sLabelTrue, sFormula;

						sResultPrec = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_RESULT_PRECISION);	
						sEnableLabelSwitching = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_ENABLE_LABEL_SWITCHING);	
						sLabelFalse = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_LABEL_FALSE);
						sLabelTrue = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_LABEL_TRUE);
						sFormula = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_LABEL_SWITCH_FORMULA);
						
						bool bOK; int iResultPrec;
						int iTemp = sResultPrec.toInt(&bOK);
						if(bOK)
							iResultPrec = iTemp;
						else
							iResultPrec = MIhmConfigGeneral::getCfg()->getDefaultResultPrecision();
						
						bool bEnableLabelSwitching = (sEnableLabelSwitching=="1")?true:false;	 
							
						pNew->initCalculationDetails(iResultPrec, 
													bEnableLabelSwitching, 
													sLabelFalse, 
													sLabelTrue, 
													sFormula);

					}
					else
					{
						if(pNew->getFormatType()==MInputExField::enuFIELD_FORMAT_UNKNOWN)
						{
							QString sMsg = QString("Invalid format for field: Type:[%1] Name:[%2] Format:[%3]!").arg(sType).arg(sName).arg(sFormat);
							TRACE_W(QString( "MInputExReq::setDefinition: %1").arg(sMsg));
							return false;
						}
					}
				}
			}
		}

		return true;
	}
	else
	{
		QString sMsg = QString("MInputExReq::setDefinition:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
		TRACE_W(sMsg);
		return false;
	}

}

MInputExField * MInputExReq::createInputField(QString sType, 
										 QString sName, 
										 QString sFormat, 
										 QString sFormatDetail, 
										 QString sMinLen,
										 QString sMaxLen,
										 QString sReadOnly,
										 QString sMultiselection,
										 QString sValidator,
										 QString sBtnId)
{
	MInputExField * pNew = new MInputExField();

	bool bReadOnly = (sReadOnly.toInt()==0)?false:true;
	bool bMultiselection = (sMultiselection.toInt()==0)?false:true;

	int iMinLen, iMaxLen;
	bool bOK;

	if(sMinLen!="")
	{
		iMinLen = sMinLen.toInt(&bOK);
		if(!bOK)
		{
			TRACE_W(QString("MInputExReq::createInputField: Error parsing sMinLen %1!")
									.arg(sMinLen));
			iMinLen = -1;
		}
	}
	else
	{
		iMinLen = -1;
	}

	if(sMaxLen!="")
	{
		iMaxLen = sMaxLen.toInt(&bOK);
		if(!bOK)
		{
			TRACE_W(QString("MInputExReq::setDescription: Error parsing sMinLen %1!")
									.arg(sMaxLen));
			iMaxLen = -1;
		}
	}
	else
	{
		iMaxLen = -1;
	}

	pNew->initDef(sType, sName, sFormat, sFormatDetail, iMinLen, iMaxLen, bReadOnly, bMultiselection, sValidator, sBtnId);

	m_lstInputFields.append(pNew);

	return pNew;
}



bool MInputExReq::setDescription(QString sDescription, int is_file)
{
	if(sDescription.size()>0)
	{
		if(is_file!=0) // check if it is file name
		{
			QString sDesFileName = sDescription;
			
			QString sFullPath = MIhmConfigGeneral::getCfg()->getCommFileFullPath(sDesFileName);

			TRACE_D(QString("MInputExReq::setDescription: File %1 !").arg(sFullPath));
			//read the file content
			sDescription = MHelpFuncs::loadFileContent(sFullPath);

			if(sDescription=="")
			{
				TRACE_W(QString("MInputExReq::setDescription: Error in file %1!").arg(sFullPath));
				m_bValid = false;
				return false;
			}
			
		}

		m_sDescription = sDescription;

		//parse description 
		m_bValid = parseDescription();
		
		if(!m_bValid)
		{
			TRACE_W("MInputExReq::setDescription: Error parsing description!");
			return false;
		}

		return true;
	}
	else
	{
		TRACE_W("MInputExReq::setDescription: Empty description!");
		m_bValid = false;
		
		return false;
	}

}

bool MInputExReq::parseDescription()
{
	if(m_sDescription=="")
		return false;
	
	QDomDocument m_xmlData;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool bRet = m_xmlData.setContent(m_sDescription, &errorStr, &errorLine, &errorColumn);

	if(bRet)
	{

		QDomElement root = m_xmlData.documentElement();
		
		if(root.tagName() != IHM_SAISIE_EX_ROOT_DESC)
		{
			TRACE_W("MInputExReq::parseDescription: Invalid root element!");
			return false;
		}

		TRACE_D("MInputExReq::parseDescription: Parsed OK!");

		QDomNodeList lstItems = root.childNodes();
		QDomNode currItem;
		QString sName, sType, sValue, sReadOnly, sSource, sTranslate;
		
		for(int i=0;i<lstItems.count();i++)
		{
			currItem = lstItems.at(i); 
			if(currItem.nodeName()==IHM_SAISIE_EX_ELEMENT)
			{
				sType = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_TYPE, true);
				sName = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_NAME, true);

				//Update the input item if it exists in the definition
				updateInputField(sType,sName,&currItem);				
			}
		}

		TRACE_D("MInputExReq::parseDescription: Fields updated OK!");


	}
	else
	{
		QString sMsg = QString("MInputExReq::parseDescription:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
		TRACE_W(sMsg);
		return false;
	}

			
	return true;
}


MInputExField * MInputExReq::getInputField(QString sName, QString sType)
{
	MInputExField * pCurrent;
	
	for (int i=0; i<m_lstInputFields.size();i++)
	{
		pCurrent = m_lstInputFields.at(i);
		

		if(pCurrent->getName() == sName)
		{
			if(sType!="") //if type set verify it
			{
				MInputExField::enuInputExFieldType eType = MInputExField::getTypeForString(sType);
				
				if(pCurrent->getType() == eType)
									return pCurrent;

			}
			else //if type not set use only the name
				return pCurrent;
		}
	}

	return NULL;
}

void MInputExReq::updateInputField(QString sType, 
										 QString sName,
										 QDomNode *pFieldNode)
{
	if(pFieldNode==NULL)
		return;

	MInputExField * pCurrent = getInputField(sName,sType);

	if(pCurrent!=NULL)
	{
		pCurrent->setValueFromXmlNode(pFieldNode);
	}

}





