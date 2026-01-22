/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 QT_translator													 */
/* FILE:	 TranslatorImpl.cpp												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include "TranslatorImpl.h"

#include <QFile>
#include <QDomDocument>
#include <QStringList>

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define XML_ELEMENT_IHM_CONFIG_ROOT			"IHM_CONFIG"
#define XML_ELEMENT_IHM_CONFIG_LANGUAGES	"LANGUAGES"
#define XML_ELEMENT_IHM_CONFIG_LANG			"LANG"
#define XML_LANG_ID							"ID"
#define XML_LANG_NATIVE_NAME				"NATIVE_NAME"
#define XML_LANG_LABEL						"LABEL"
#define XML_LANG_LABEL_ID					"ID"
#define XML_LANG_LABEL_TRANSLATION			"TRANSLATION"

#define STR_NEWLINE_FOR_DSK "\n"
#define IHM_STR_NEW_LINE "\xF5"

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*---------------------------------- CODE: ----------------------------------*/

TranslatorImpl::TranslatorImpl():
isInitialized(false),
m_plstSelectedLanguage(NULL)
{
	
}

TranslatorImpl::~TranslatorImpl()
{
	while(!m_lstAllLanguages.isEmpty())
		delete m_lstAllLanguages.takeLast();
}


TranslatorImpl::InitConstants TranslatorImpl::initLanguagesFromFile(QString langFilePath, QString language, QString systemEncoding)
{
	TranslatorImpl::InitConstants bRetVal = IMPL_FAIL;
	m_sInitErrorString = "";

	m_lstLangIDs.clear();
	m_lstLangNativeNames.clear();
	while(!m_lstAllLanguages.isEmpty())
		delete m_lstAllLanguages.takeLast();

	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument xmlData;

	QFile file(langFilePath);
	
	if(file.open(QFile::ReadOnly))
	{
		bool bRet = xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn);
		if(bRet)
		{
			QDomElement root = xmlData.documentElement();
			if(root.tagName() != XML_ELEMENT_IHM_CONFIG_ROOT)
			{
				m_sInitErrorString = "Invalid root element!";
				bRet = IMPL_FAIL;
			}
			else
			{
				QDomNodeList lstLanguagesItems = root.elementsByTagName(XML_ELEMENT_IHM_CONFIG_LANGUAGES);
				
				if(lstLanguagesItems.count()>0)
				{
					QDomNode nodeLanguages = lstLanguagesItems.at(0);

					//root element for all lane types
					QDomElement elLanguages = nodeLanguages.toElement();
					QDomNodeList lstLangItems = elLanguages.elementsByTagName(XML_ELEMENT_IHM_CONFIG_LANG);
					
					QDomNode currLang; 
					
					LanguageList * pNewLanguageList;

					for(int i=0;i<lstLangItems.count();i++)
					{
						currLang = lstLangItems.at(i); 

						QString sID = TranslatorImpl::getAttributeText(&currLang, XML_LANG_ID);
						QString sNativeName = TranslatorImpl::getAttributeText(&currLang, XML_LANG_NATIVE_NAME);

						pNewLanguageList = new LanguageList(sID);

						m_lstAllLanguages.append(pNewLanguageList);

						m_lstLangIDs.append(sID);
						if(sNativeName.isEmpty())
							m_lstLangNativeNames.append(sID);
						else
							m_lstLangNativeNames.append(sNativeName);

						QDomNodeList lstLabelItems = currLang.toElement().elementsByTagName(XML_LANG_LABEL);
					
						QDomNode currLabel; 
						
						for(int i=0;i<lstLabelItems.count();i++)
						{
							currLabel = lstLabelItems.at(i); 

							QString sLabelID = getAttributeText(&currLabel, XML_LANG_LABEL_ID);
							QString sLabelTranslation = getAttributeText(&currLabel, XML_LANG_LABEL_TRANSLATION);
							
							if(sLabelID!="" && sLabelTranslation!="")
							{
								LanguageLabel *pNewLabel = new LanguageLabel();
								pNewLabel->m_sLabelName = sLabelID;
								pNewLabel->m_sLabelTranslation = sLabelTranslation;

								pNewLanguageList->append(pNewLabel);
								bRetVal = IMPL_SUCCESS;
							}
						}
						pNewLanguageList->sort();
					}
				}
				else
				{
					m_sInitErrorString = QString("Element %s not found!").arg(XML_ELEMENT_IHM_CONFIG_LANGUAGES);
					return IMPL_FAIL;
				}
			}
		}
		else
		{
			m_sInitErrorString = QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
			return IMPL_FAIL;
		}
	}
	else
	{
		m_sInitErrorString = QString( "Unable to open file %1").arg(langFilePath) ;
		return IMPL_FAIL;
	}

	if(bRetVal == IMPL_SUCCESS)
	{
		if(!setLanguage(language))
		{
			 m_sInitErrorString = QString( "Error could not find requested language %1").arg(language);
		
			 if (m_lstAllLanguages.size() > 0)
			 {	
				m_sSelectedLanguage = m_lstAllLanguages.at(0)->getLanguageID();
				m_plstSelectedLanguage = m_lstAllLanguages.at(0);
				bRetVal = IMPL_NO_REQ_LANGUAGE;
			 }
			 else
			 {
				m_sInitErrorString = "Error: No language found!";
				m_plstSelectedLanguage = NULL;
				bRetVal = IMPL_FAIL;
			 }
		}
		else
			bRetVal = IMPL_SUCCESS;
	}

	isInitialized = true;
	return bRetVal;
}

bool TranslatorImpl::setLanguage(QString sLanguageID)
{
	bool bRetVal = false;

	for (int i = 0; i < m_lstAllLanguages.size(); ++i) 
	{
		if ( sLanguageID.compare(m_lstAllLanguages.at(i)->getLanguageID(), Qt::CaseInsensitive) == 0)
		{	
			m_sSelectedLanguage = m_lstAllLanguages.at(i)->getLanguageID();
			m_plstSelectedLanguage = m_lstAllLanguages.at(i);
			
			bRetVal = true;
			break;
		}
	}

	return bRetVal;
}

bool TranslatorImpl::setLanguage(int index)
{
	if(index < 0 || index >= m_lstAllLanguages.size())
		return false;

	m_sSelectedLanguage = m_lstAllLanguages.at(index)->getLanguageID();
	m_plstSelectedLanguage = m_lstAllLanguages.at(index);

	return true;
}

int TranslatorImpl::getCurrentLangIndex()
{
	int retVal = -1;
	for (int i = 0; i < m_lstAllLanguages.size(); ++i)
	{
		if (m_lstAllLanguages.at(i)->getLanguageID() == m_sSelectedLanguage)
		{
			retVal = i;
			break;
		}
	}

	return retVal;
}

void TranslatorImpl::setSystemEncoding(QString encoding)
{

}

QString TranslatorImpl::getLabelTranslation(QString label) const
{
	QString sRetVal;
	bool bFound = false;
	
	if(m_plstSelectedLanguage!=NULL)
	{
		bFound = m_plstSelectedLanguage->getTranslation(label, sRetVal);
	}
	
	if(!bFound)
	{
		sRetVal = label;
	}
	
	return replaceNewLinesForDsk(sRetVal);
}

QString TranslatorImpl::translate(const char * context, const char * sourceText, const char * disambiguation) const
{
	return getLabelTranslation(sourceText);
}

QString TranslatorImpl::getAttributeText(QDomNode *pNode, QString sAttName, bool bTrim)
{
	QString sText;
	
	QDomNode nodAtt= pNode->attributes().namedItem(sAttName);
	
	if(!nodAtt.isNull())
	{
		sText = nodAtt.nodeValue();
		
		if(bTrim)
			sText = sText.trimmed();
	}
	
	return sText;
}

QString TranslatorImpl::replaceNewLinesForDsk(QString str)
{
	return str.replace(IHM_STR_NEW_LINE,STR_NEWLINE_FOR_DSK);
}

