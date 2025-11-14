#include <QDomDocument>
#include <qregexp.h>

#include "MInputExField.h"
#include "MHelpFuncs.h"
#include "MIhmConfigGeneral.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
	#include <reg.h>
}


MOptionData& MOptionData::operator=(const MOptionData& right)
{
	this->m_sText = MHelpFuncs::deepCopy(right.m_sText);
	this->m_sValue = MHelpFuncs::deepCopy(right.m_sValue);
	
	return *this;
}



MInputExField::MInputExField()
{
	m_iNumericPreDecimals = 0;
	m_iNumericDecimals = 0;
	m_bNumericAllowNegative = false;
	m_bNumericLeftPadZeroes = false;
	m_chNumericSeparatorChar = ',';
	m_iResultPrec = 2;
	m_sKeyID = "";

	m_bIsReadOnly = false;
	m_iMinLen = -1;
	m_iMaxLen = -1;
	m_bMultiselection = false;
	
	//Calculation Details 
	m_bEnableLabelSwitching = false;  

	//mask input properties
	m_sMaskPlaceholder = "_";
	m_bIsValueHtml = false;

	m_eFormat = enuFIELD_FORMAT_UNKNOWN;
}

MInputExField::~MInputExField()
{
	while (!m_lstOptions.isEmpty())
		delete m_lstOptions.takeFirst();
}

MInputExField::MInputExField(MInputExField &obj)
{
	*this = obj;
}



MInputExField& MInputExField::operator=(const MInputExField& right)
{
	this->m_eType = right.m_eType;
	this->m_sName = MHelpFuncs::deepCopy(right.m_sName);
	this->m_sValue = MHelpFuncs::deepCopy(right.m_sValue);
	this->m_bIsValueHtml = right.m_bIsValueHtml;

	this->m_sText = MHelpFuncs::deepCopy(right.m_sText);
	this->m_bIsReadOnly = right.m_bIsReadOnly;
	this->m_sFormat = MHelpFuncs::deepCopy(right.m_sFormat);
	this->m_eFormat = right.m_eFormat;
	this->m_sFormatDetail = MHelpFuncs::deepCopy(right.m_sFormatDetail);
	//this->m_sXml = right.m_sXml;
	this->m_iMinLen = right.m_iMinLen;
	this->m_iMaxLen = right.m_iMaxLen;
	this->m_bMultiselection = right.m_bMultiselection;
	this->m_sSecondaryValidator = MHelpFuncs::deepCopy(right.m_sSecondaryValidator);
	this->m_sKeyID = right.m_sKeyID;

	//Calculation Details
	this->m_iResultPrec = right.m_iResultPrec; 
	this->m_bEnableLabelSwitching = right.m_bEnableLabelSwitching;
	this->m_sLabelFalse = MHelpFuncs::deepCopy(right.m_sLabelFalse); 
	this->m_sLabelTrue = MHelpFuncs::deepCopy(right.m_sLabelTrue);
	this->m_sLblSwitchFormula = MHelpFuncs::deepCopy(right.m_sLblSwitchFormula);	

	// numeric input properties
	this->m_iNumericPreDecimals = right.m_iNumericPreDecimals;
	this->m_iNumericDecimals = right.m_iNumericDecimals;
	this->m_bNumericAllowNegative = right.m_bNumericAllowNegative;
	this->m_bNumericLeftPadZeroes = right.m_bNumericLeftPadZeroes;
	this->m_chNumericSeparatorChar = right.m_chNumericSeparatorChar;

	//mask input properties
	this->m_sInputMask = MHelpFuncs::deepCopy(right.m_sInputMask);
	this->m_sMaskPlaceholder = MHelpFuncs::deepCopy(right.m_sMaskPlaceholder);	

	while (!m_lstOptions.isEmpty())
		delete m_lstOptions.takeFirst();

	MOptionData * pCurrent;

	for(int i=0;i<right.m_lstOptions.size();i++)
	{
		pCurrent = right.m_lstOptions.at(i);

		MOptionData * pNew = new MOptionData();

		if(pCurrent!=NULL)
			*pNew = *pCurrent;
		
		m_lstOptions.append(pNew);
	}

	return *this;	
}


MInputExField::enuInputExFieldType MInputExField::getTypeForString(QString sType)
{
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_CHK_BOX, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_CHECK_BOX;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_RADIO_BTN, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_RADIO_BUTTON;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_COMBO_BOX, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_COMBO_BOX;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_LIST_WIDGET, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_LIST_WIDGET;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_LINE_EDIT, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_LINE_EDIT;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_LINE_EDIT_EXT, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_LINE_EDIT_EXT;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_HIDDEN_EDIT, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_HIDDEN_EDIT;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_TEXT_EDIT, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_TEXT_EDIT;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_LABEL, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_LABEL;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_LABEL_LUHN_CHECK, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_LABEL_LUHN_CHECK;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_BUTTON_OK, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_BUTTON_OK;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_BUTTON_CANCEL, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_BUTTON_CANCEL;
	if(QString::compare(sType, IHM_SAISIE_EX_INPUT_BUTTON, Qt::CaseInsensitive)==0)
		return enuDLG_EX_FIELD_BUTTON;

	return enuDLG_EX_FIELD_UNKNOWN;
}


QString MInputExField::getStringForType(enuInputExFieldType eType)
{
	switch(eType)
	{
		case enuDLG_EX_FIELD_CHECK_BOX:
			return QString(IHM_SAISIE_EX_INPUT_CHK_BOX);
		case enuDLG_EX_FIELD_RADIO_BUTTON:
			return QString(IHM_SAISIE_EX_INPUT_RADIO_BTN);
		case enuDLG_EX_FIELD_COMBO_BOX:
			return QString(IHM_SAISIE_EX_INPUT_COMBO_BOX);
		case enuDLG_EX_FIELD_LIST_WIDGET:
			return QString(IHM_SAISIE_EX_INPUT_LIST_WIDGET);
		case enuDLG_EX_FIELD_LINE_EDIT:
			return QString(IHM_SAISIE_EX_INPUT_LINE_EDIT);
		case enuDLG_EX_FIELD_LINE_EDIT_EXT:
			return QString(IHM_SAISIE_EX_INPUT_LINE_EDIT_EXT);
		case enuDLG_EX_FIELD_HIDDEN_EDIT:
			return QString(IHM_SAISIE_EX_INPUT_HIDDEN_EDIT);
		case enuDLG_EX_FIELD_TEXT_EDIT:
			return QString(IHM_SAISIE_EX_INPUT_TEXT_EDIT);
		case enuDLG_EX_FIELD_LABEL:
			return QString(IHM_SAISIE_EX_INPUT_LABEL);
		case enuDLG_EX_FIELD_LABEL_LUHN_CHECK:
			return QString(IHM_SAISIE_EX_INPUT_LABEL_LUHN_CHECK);
		case enuDLG_EX_FIELD_BUTTON_OK:
			return QString(IHM_SAISIE_EX_INPUT_BUTTON_OK);
		case enuDLG_EX_FIELD_BUTTON_CANCEL:
			return QString(IHM_SAISIE_EX_INPUT_BUTTON_CANCEL);
		case enuDLG_EX_FIELD_BUTTON:
			return QString(IHM_SAISIE_EX_INPUT_BUTTON);

		default:
			return QString("");
	}
}

MInputExField::enuInputExFieldFormatType MInputExField::getFormatEnumFromString(QString sFormat)
{
	if(QString::compare(sFormat, IHM_SAISIE_EX_FIELD_FORMAT_FILTER, Qt::CaseInsensitive)==0)
		return enuFIELD_FORMAT_FILTER;
	if (QString::compare(sFormat, IHM_SAISIE_EX_FIELD_FORMAT_FILTER_BEGINS_WITH, Qt::CaseInsensitive) == 0)
			return enuFIELD_FORMAT_FILTER_BEGINS_WITH;
	if(QString::compare(sFormat, IHM_SAISIE_EX_FIELD_FORMAT_LUHN_KEY, Qt::CaseInsensitive)==0)
		return enuFIELD_FORMAT_LUHN_KEY;
	if(QString::compare(sFormat, IHM_SAISIE_EX_FIELD_FORMAT_LUHN_KEY_MOD16, Qt::CaseInsensitive)==0)
		return enuFIELD_FORMAT_LUHN_KEY_MOD16;
	if(QString::compare(sFormat, IHM_SAISIE_EX_FIELD_FORMAT_LUHN_KEY_MOD16_M3, Qt::CaseInsensitive)==0)
		return enuFIELD_FORMAT_LUHN_KEY_MOD16_M3;
	if(QString::compare(sFormat, IHM_SAISIE_EX_FIELD_FORMAT_NUMBER, Qt::CaseInsensitive)==0)
		return enuFIELD_FORMAT_NUMBER;
	if(QString::compare(sFormat.left(1), IHM_SAISIE_EX_FIELD_FORMAT_TEXT, Qt::CaseInsensitive)==0)
		return enuFIELD_FORMAT_TEXT;
	if(QString::compare(sFormat, IHM_SAISIE_EX_FIELD_FORMAT_MASK, Qt::CaseInsensitive)==0)
		return enuFIELD_FORMAT_MASK;
	if(QString::compare(sFormat, IHM_SAISIE_EX_FIELD_FORMAT_RIGHTMASK, Qt::CaseInsensitive)==0)
		return enuFIELD_FORMAT_RIGHTMASK;
	if(QString::compare(sFormat, IHM_SAISIE_EX_FIELD_FORMAT_CALCULATION, Qt::CaseInsensitive)==0)
		return enuFIELD_FORMAT_CALCULATION;

	return enuFIELD_FORMAT_UNKNOWN;
}


bool MInputExField::hasReturningValue()
{
	switch(m_eType)
	{
		case enuDLG_EX_FIELD_CHECK_BOX:
		case enuDLG_EX_FIELD_RADIO_BUTTON:
		case enuDLG_EX_FIELD_COMBO_BOX:
		case enuDLG_EX_FIELD_LIST_WIDGET:
		case enuDLG_EX_FIELD_LINE_EDIT:
		case enuDLG_EX_FIELD_LINE_EDIT_EXT:
		case enuDLG_EX_FIELD_TEXT_EDIT:
			return true;
	}

	return false;
}

void MInputExField::initDef(QString sType, 
							QString sName, 
							QString sFormat, 
							QString sFormatDetail, 
							int iMinLen, 
							int iMaxLen, 
							bool bReadOnly,
							bool bMultiselection,
							QString sValidator,
							QString sKeyID)
{
	m_sName = sName;
	m_eType = MInputExField::getTypeForString(sType);

	m_bIsReadOnly = bReadOnly;
	m_sFormat = sFormat;
	m_eFormat = MInputExField::getFormatEnumFromString(sFormat);
	m_sFormatDetail = sFormatDetail;
	m_iMinLen = iMinLen;
	m_iMaxLen = iMaxLen;
	m_bMultiselection = bMultiselection;
	m_sSecondaryValidator = sValidator;
	m_sKeyID = sKeyID; 

	// numeric input properties - default initialization
	if(m_eFormat == enuFIELD_FORMAT_NUMBER)
		parseNumberFormatStr(sFormatDetail);
	else if((m_eFormat == enuFIELD_FORMAT_MASK || m_eFormat == enuFIELD_FORMAT_RIGHTMASK) 
				&& sFormatDetail!="")
	{
		QStringList sLst = sFormatDetail.split(INPUT_MASK_PLACEHOLDER_SEPARATOR);
		
		m_sInputMask = "";
		m_sMaskPlaceholder = " ";

		if(sLst.size()>0)
		{
			m_sInputMask = sLst.at(0);
		}

		if(sLst.size()>1)
			m_sMaskPlaceholder = sLst.at(1);
	}	


}



void MInputExField::initCalculationDetails(int iResultPrec, 
								bool bEnableLabelSwitching, 
								QString sLabelFalse, 
								QString sLabelTrue, 
								QString sLblSwitchFormula)
{
	m_iResultPrec = iResultPrec; 
	m_bEnableLabelSwitching = bEnableLabelSwitching; 
	
	m_sLabelFalse = sLabelFalse; 
	m_sLabelTrue = sLabelTrue; 
	m_sLblSwitchFormula = sLblSwitchFormula;	
}


bool MInputExField::setValueFromXmlNode(QDomNode *pFieldNode)
{
	QString sName, sType, sReadOnly,sValue,sText;

	sType = MHelpFuncs::getAttributeText(pFieldNode, IHM_SAISIE_EX_ATT_TYPE, true);
	sName = MHelpFuncs::getAttributeText(pFieldNode, IHM_SAISIE_EX_ATT_NAME, true);
	
	sReadOnly = MHelpFuncs::getAttributeText(pFieldNode, IHM_SAISIE_EX_ATT_READONLY);
	m_bIsReadOnly = (sReadOnly=="0"||sReadOnly=="")?false:true;

	m_sValue = MHelpFuncs::getAttributeText(pFieldNode, IHM_SAISIE_EX_ATT_VALUE);
	m_sText = MHelpFuncs::getAttributeText(pFieldNode, IHM_SAISIE_EX_ATT_TEXT);
	
	
	TRACE_D(QString( "MInputExField::setXMLValue:Updaing xml for field %1 named %2!")
						.arg(sType)
						.arg(m_sName));

	
	if(m_eType == enuDLG_EX_FIELD_LINE_EDIT_EXT || m_eType == enuDLG_EX_FIELD_LINE_EDIT)
	{	
		//Write a warning why the text value is not visible (to ease the configuration) 
		if(m_sValue=="" && m_sText!="")
		{
			QString sMsg = QString("MInputDialogExReq::setXMLValue: Warning: Field %1: Note that attribute \"%2\" should be used for the control content text and not attribute \"%3\"!")
							.arg(sName).arg(IHM_SAISIE_EX_ATT_VALUE).arg(IHM_SAISIE_EX_ATT_TEXT);
			TRACE_W(sMsg);

		}

	}
	else if(m_eType == enuDLG_EX_FIELD_TEXT_EDIT)
	{
		QString sSourceFile = MHelpFuncs::getAttributeText(pFieldNode, IHM_SAISIE_EX_ATT_SOURCE);
		QString sIsHtml = MHelpFuncs::getAttributeText(pFieldNode, IHM_SAISIE_EX_ATT_IS_HTML);
		
		m_bIsValueHtml = (sIsHtml == "1")?true:false; 

		if(sSourceFile!="")
		{
			QString sFullPath = MIhmConfigGeneral::getCfg()->getCommFileFullPath(sSourceFile);
			m_sValue = MHelpFuncs::loadFileContent(sFullPath);
		}

		//Write a warning why the text value is not visible (to ease the configuration) 
		if(m_sValue=="" && m_sText!="")
		{
			QString sMsg = QString("MInputDialogExReq::setXMLValue: Warning: Field %1: Note that attribute \"%2\" should be used for the control content text and not attribute \"%3\"!")
							.arg(sName).arg(IHM_SAISIE_EX_ATT_VALUE).arg(IHM_SAISIE_EX_ATT_TEXT);
			TRACE_W(sMsg);
		}

	}
	else if(m_eType == enuDLG_EX_FIELD_COMBO_BOX ||
				m_eType == enuDLG_EX_FIELD_LIST_WIDGET) //try load options only for combo boxes and list widget
	{
		
		QString sSourceFile = MHelpFuncs::getAttributeText(pFieldNode, IHM_SAISIE_EX_ATT_SOURCE);
		
		QDomNodeList lstItems = pFieldNode->childNodes();

		if(lstItems.count()>0 || sSourceFile!="")
		{
			while (!m_lstOptions.isEmpty())
				delete m_lstOptions.takeFirst();
			
			if(sSourceFile!="")	//add options from file that the widget element reference with src attribute
			{
				return loadOptionsFromFile(sSourceFile);			
			}			
			else //or add options received in the description
			{
				QDomNode currItem;

				for(int i=0;i<lstItems.count();i++)
				{
					currItem = lstItems.at(i); 
					if(currItem.nodeName()==IHM_SAISIE_EX_LIST_OPTION)
					{
						sText = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_TEXT);
						sValue = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_VALUE);
						
						MOptionData * pNew = new MOptionData();

						pNew->m_sText = sText;
						pNew->m_sValue = sValue;
						m_lstOptions.append(pNew);
					}
				}

				return true;
			}
		
		}
	} // END try load options only for combo boxes and list widget

	return true;
}


QString MInputExField::getXMLValue()
{
	QString sXML;
	
	if(m_eType==enuDLG_EX_FIELD_CHECK_BOX ||
		m_eType==enuDLG_EX_FIELD_RADIO_BUTTON ||
		m_eType==enuDLG_EX_FIELD_COMBO_BOX ||
		m_eType==enuDLG_EX_FIELD_LIST_WIDGET ||
		m_eType==enuDLG_EX_FIELD_LINE_EDIT ||
		m_eType==enuDLG_EX_FIELD_TEXT_EDIT )
	{
		sXML += QString("<%1").arg(IHM_SAISIE_EX_ELEMENT);
		sXML += QString(" %1=\"%2\" ").arg(IHM_SAISIE_EX_ATT_TYPE).arg(getStringForType(m_eType));
		sXML += QString(" %1=\"%2\" ").arg(IHM_SAISIE_EX_ATT_NAME).arg(m_sName);
		sXML += QString(" %1=\"%2\" ").arg(IHM_SAISIE_EX_ATT_READONLY).arg((m_bIsReadOnly)?"true":"false");
		sXML += QString(" %1=\"%2\" ").arg(IHM_SAISIE_EX_ATT_VALUE).arg(m_sValue);
		sXML += QString(" %1=\"%2\" ").arg(IHM_SAISIE_EX_ATT_TEXT).arg(m_sText);

		sXML += "/>";
	}

	return sXML;
}




bool MInputExField::loadOptionsFromFile(QString sSourceFile)
{
	QString sFullPath = MIhmConfigGeneral::getCfg()->getCommFileFullPath(sSourceFile);
	QString sXml = MHelpFuncs::loadFileContent(sFullPath);
	
	TRACE_D(QString( "MInputExField::loadOptionsFromFile: ...."));
	
	if(sXml=="")
	{
		TRACE_W(QString( "MInputExField::loadOptionsFromFile: Unable to load options list file %1").arg(sFullPath));
		return false;
	}

	QDomDocument m_xmlData;
	QString errorStr;
	int errorLine;
	int errorColumn;

	bool bRet = m_xmlData.setContent(sXml, &errorStr, &errorLine, &errorColumn);

	if(bRet)
	{
		QDomElement root = m_xmlData.documentElement();
		if(root.tagName() != IHM_SAISIE_EX_LIST_OPT_ROOT)
		{
			TRACE_W(QString( "MInputExField::loadOptionsFromFile:Invalid root element %1 for file %2!").
									arg(IHM_SAISIE_EX_LIST_OPT_ROOT).
									arg(sFullPath));
			return false;
		}
		

		//add options received in the description
		QString sValue,sText;
		QDomNodeList lstItems = root.childNodes();
		QDomNode currItem;

		for(int i=0;i<lstItems.count();i++)
		{
			currItem = lstItems.at(i); 
			if(currItem.nodeName()==IHM_SAISIE_EX_LIST_OPTION)
			{
				sText = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_TEXT);
				sValue = MHelpFuncs::getAttributeText(&currItem, IHM_SAISIE_EX_ATT_VALUE);
				
				MOptionData * pNew = new MOptionData();

				pNew->m_sText = sText;
				pNew->m_sValue = sValue;
				m_lstOptions.append(pNew);
			}
		}


	}
	else
	{
		QString sMsg = QString("MInputExField::loadOptionsFromFile:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
		TRACE_W(sMsg);
		return false;
	}	

	return true;
}		

bool MInputExField::parseNumberFormatStr(QString sNumInputFormat)
{
	int iSize = sNumInputFormat.size();
	
	if(iSize<=1) 
			return false;
	
	QString sPart;
	m_bNumericAllowNegative = false;
	m_bNumericLeftPadZeroes = false;
	m_iNumericPreDecimals = 0;
	m_iNumericDecimals = 0;
	m_chNumericSeparatorChar = MIhmConfigGeneral::getCfg()->getDefaultDecimalSeparator();

	int iPos = 1;
	QChar chCurrent = sNumInputFormat.at(iPos);

	if(chCurrent == IHM_FIELD_FORMAT_CHAR_SIGNED)
	{
		iPos++;
		m_bNumericAllowNegative = true;
		if(iSize<=iPos) 
				return false;
		chCurrent = sNumInputFormat.at(iPos);
	}
	
	if(chCurrent == IHM_FIELD_FORMAT_CHAR_ZERO)
	{
		m_bNumericLeftPadZeroes = true;
		iPos++;
		
		if(iSize<=iPos) 
			return false;
		chCurrent = sNumInputFormat.at(iPos);
	}

	sPart = "";
	
	while(chCurrent.isDigit())
	{
		sPart += chCurrent;
		iPos++;
		if(iSize<=iPos) 
			break;
		chCurrent = sNumInputFormat.at(iPos);
	}

	bool bOK;
	QString sMask2;

	m_iNumericPreDecimals = sPart.toInt(&bOK);

	if(!bOK)
		return false;

	if(iSize<=iPos) 
	{
		m_iNumericDecimals=0;
		return true;  
	}

	iPos++; //skip decimal separator and use default one from MIhmConfigGeneral
	
	if(iSize<=iPos) 
			return false;
	
	chCurrent = sNumInputFormat.at(iPos);

	sPart = "";
	while(chCurrent.isDigit())
	{
		sPart += chCurrent;
		
		iPos++;
		if(iSize<=iPos) 
			break;
		chCurrent = sNumInputFormat.at(iPos);
	}

	m_iNumericDecimals = sPart.toInt(&bOK);
	
	if(!bOK)
		return false;

	return true;
}
