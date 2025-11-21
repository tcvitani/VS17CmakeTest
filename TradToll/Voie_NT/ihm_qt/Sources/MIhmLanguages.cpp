

#include <QString>
#include <QDomDocument>
#include <QFile>
#include "MHelpFuncs.h"
#include "MIhmConfigWeb.h"


#include "MIhmLanguages.h"
#include "MTracer.h"

extern "C" {
	#include <run.H>
	#include <ihm.H>
	#include <reg.h>
};


#define XML_ELEMENT_IHM_CONFIG_ROOT			"IHM_CONFIG"
#define XML_ELEMENT_IHM_CONFIG_LANGUAGES	"LANGUAGES"
#define XML_ELEMENT_IHM_CONFIG_LANG			"LANG"
#define XML_LANG_ID							"ID"
#define XML_LANG_LABEL						"LABEL"
#define XML_LANG_LABEL_ID					"ID"
#define XML_LANG_LABEL_TRANSLATION			"TRANSLATION"
 

//--------------------------------------------------------------

MIhmLanguages::MIhmLanguages()
{
	m_plstSelectedLanguage = NULL;
}


MIhmLanguages::~MIhmLanguages()
{
	while(!m_lstAllLanguages.isEmpty())
		delete m_lstAllLanguages.takeLast();

}

bool MIhmLanguages::initFromFile(QString sLangFilePath, QString sDefltLanguage, bool bUsedForWeb)
{
	bool bRetVal = false;
    TRACE_D(QString("MIhmLanguages::initFromFile..file:[%1],language[%2]")
									.arg(sLangFilePath)
									.arg(sDefltLanguage));
	
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument xmlData;

	QFile file(sLangFilePath);
	
	if(file.open(QFile::ReadOnly))
	{

		bool bRet = xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn);

		if(bRet)
		{
			QDomElement root = xmlData.documentElement();
			if(root.tagName() != XML_ELEMENT_IHM_CONFIG_ROOT)
			{
				TRACE_W("MIhmLanguages::initFromFile: Invalid root element!");
				bRet = false;
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

						QString sID = MHelpFuncs::getAttributeText(&currLang, XML_LANG_ID);

						pNewLanguageList = new LanguageList();
						pNewLanguageList->m_sLanguageName = sID;
						pNewLanguageList->m_plstLanguage = new QList<LanguageLabel*>();

						m_lstAllLanguages.append(pNewLanguageList);

						
						QDomNodeList lstLabelItems = currLang.toElement().elementsByTagName(XML_LANG_LABEL);
					
						QDomNode currLabel; 
						
						for(int i=0;i<lstLabelItems.count();i++)
						{
							currLabel = lstLabelItems.at(i); 

							QString sLabelID = MHelpFuncs::getAttributeText(&currLabel, XML_LANG_LABEL_ID);
							QString sLabelTranslation = MHelpFuncs::getAttributeText(&currLabel, XML_LANG_LABEL_TRANSLATION);
							
							if(sLabelID!="" && sLabelTranslation!="")
							{
								LanguageLabel *pNewLabel = new LanguageLabel();
								pNewLabel->m_sLabelName = sLabelID;
								pNewLabel->m_sLabelTranslation = sLabelTranslation;

								//eTranslType == enuTranslTargetDesktop
								pNewLabel->m_sLabelTranslationForDsk = MHelpFuncs::replaceNewLinesForDsk(sLabelTranslation);
								
								if(bUsedForWeb)
								{
									//eTranslType == enuTranslTargetWeb
									pNewLabel->m_sLabelTranslationForWeb = MIhmConfigWeb::getCfg()->htmlEncodeStr(sLabelTranslation);
									pNewLabel->m_sLabelTranslationForWeb = MHelpFuncs::replaceNewLinesForWeb(pNewLabel->m_sLabelTranslationForWeb);
								}

								pNewLanguageList->m_plstLanguage->append(pNewLabel);
								bRetVal = true;
							}

						}
						

						pNewLanguageList->sort();
						
					}

					
				}
				else
				{
					TRACE_W(QString( "MIhmLanguages::initFromFile: Element %1 not found!")
									.arg(XML_ELEMENT_IHM_CONFIG_LANGUAGES));
					return false;
				}

			}
		}
		else
		{
			QString sMsg = QString("MIhmLanguages::initFromFile:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
			TRACE_W(sMsg);
			return false;
		}
	}
	else
	{
		TRACE_W(QString( "MIhmLanguages::initFromFile: Unable to open file %1")
									.arg(sLangFilePath) );
		return false;
	}

	
 
    
		
	if(bRetVal==true)
	{
		if(!setLanguage(sDefltLanguage))
		{
			 TRACE_W(QString( "MIhmLanguages::initialize: Error could not find default language %1")
									.arg(sDefltLanguage) );
		
			 if (m_lstAllLanguages.size() > 0)
			 {	
				m_sSelectedLanguage = m_lstAllLanguages.at(0)->m_sLanguageName;
				m_plstSelectedLanguage = m_lstAllLanguages.at(0);
				bRetVal = true;

			 }
			 else
			 {
				TRACE_W("MIhmLanguages::initialize: Error: No language found!");
				m_plstSelectedLanguage = NULL;
			 }
		}
		else
			bRetVal = true;

	}
	

	return bRetVal;
}



bool MIhmLanguages::setLanguage(QString sLanguage)
{
	bool bRetVal = false;

	m_sSelectedLanguage = sLanguage;
	m_plstSelectedLanguage = NULL;

	for (int i = 0; i < m_lstAllLanguages.size(); ++i) 
	{
		 if (m_lstAllLanguages.at(i)->m_sLanguageName == sLanguage)
		 {	
			m_sSelectedLanguage = m_lstAllLanguages.at(i)->m_sLanguageName;
			m_plstSelectedLanguage = m_lstAllLanguages.at(i);

			bRetVal = true;
			break;
		 }
	 }

	return bRetVal;
}

bool MIhmLanguages::getLabelTranslation(QString sLabelName, enumTranslTarget eTranslType, QString &sRetVal)
{
	bool bFound = false; 

	if(!sLabelName.isEmpty())
	{
		if(m_plstSelectedLanguage!=NULL)
		{
			bFound = m_plstSelectedLanguage->getTranslation(sLabelName, eTranslType, sRetVal);
		}

		if(!bFound)
		{
			sRetVal = sLabelName;

			TRACE_D(QString("MIhmLanguages::getLabelTranslation: Translation not found for:[%1]").arg(sLabelName));

			if(eTranslType == enuTranslTargetDesktop)
			{
				sRetVal = MHelpFuncs::replaceNewLinesForDsk(sRetVal);
			}
			else if(eTranslType == enuTranslTargetWeb)
			{
				sRetVal = MIhmConfigWeb::getCfg()->htmlEncodeStr(sRetVal);
				sRetVal = MHelpFuncs::replaceNewLinesForWeb(sRetVal);
			}
		}
	}
	else
		sRetVal = sLabelName;

	return bFound;
}

QString MIhmLanguages::getOKButtonTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_OK_TEXT, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getCancelButtonTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_CANCEL_TEXT, eTranslType, sResult);
	return sResult;

}


QString MIhmLanguages::getLoginOKButtonTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_LOGIN_OK_TEXT, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getLoginCancelButtonTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_LOGIN_CANCEL_TEXT, eTranslType, sResult);
	return sResult;

}



QString MIhmLanguages::getClientCloseButtonTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_CLIENT_CLOSE_TEXT, eTranslType, sResult);
	return sResult;
}


//TAKE OVER LABELS AND TRANSLATIONS ....

QString MIhmLanguages::getTakeOverAcceptButtonTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_TAKE_OVER_ACCEPT, eTranslType, sResult);
	return sResult;

}

QString MIhmLanguages::getTakeOverRejectButtonTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_TAKE_OVER_REJECT, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getLoginTitleTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_TITLE_LOGIN_DIALOG, eTranslType, sResult);
	return sResult;

}

QString MIhmLanguages::getLoginMessageTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_LOGIN_DIALOG_MESSAGE, eTranslType, sResult);
	return sResult;
}




QString MIhmLanguages::getLabelErrMsgTooManyUsersConnected(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_ERR_MSG_TOO_MANY_USERS_CONNECTED, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getLabelErrSessionIdleTimeout(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_ERR_SESSION_IDLE_TIMEOUT, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getLabelErrSessionNotFound(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_ERR_SESSION_NOT_FOUND, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getLoginLDAPOfflineMessageTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_LOGIN_DIALOG_MESSAGE_LDAP_OFFLINE, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getLoginNameTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_LOGIN_DIALOG_NAME, eTranslType, sResult);
	return sResult;

}

QString MIhmLanguages::getLoginPwdTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_LOGIN_DIALOG_PWD, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getLoginSecNumberTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_LOGIN_DIALOG_SEC_NUM, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getTakeOverQuestionTitleTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_TITLE_TAKE_OVER_DIALOG_QUESTION, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getTakeOverQuestionTranslation(enumTranslTarget eTranslType, QString sUserName)
{
	QString sQuestMsg;
	
	getLabelTranslation(IHM_LABEL_TAKE_OVER_DIALOG_QUESTION, enuTranslTargetNoTransform, sQuestMsg);

	sQuestMsg = QString(sQuestMsg).arg(sUserName);

	if(eTranslType == enuTranslTargetDesktop)
	{
		sQuestMsg = MHelpFuncs::replaceNewLinesForDsk(sQuestMsg);
	}
	else if(eTranslType == enuTranslTargetWeb)
	{
		sQuestMsg = MIhmConfigWeb::getCfg()->htmlEncodeStr(sQuestMsg);
		sQuestMsg = MHelpFuncs::replaceNewLinesForWeb(sQuestMsg);
	}
	
	return sQuestMsg;
}


QString MIhmLanguages::getTakeOverInfoTitleTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_TAKE_OVER_WAITING_DLG_TITLE, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getTakeOverInfoTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_TAKE_OVER_WAITING_DLG_MSG, eTranslType, sResult);
	return sResult;
}


QString MIhmLanguages::getTakeCtrlFailureDlgTitleTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_TAKE_CTRL_FAILURE_DLG_TITLE, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getTakeCtrlRejectedByUserMsgTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_TAKE_CTRL_REJECTED_BY_USER_MESSAGE, eTranslType, sResult);
	return sResult;
}


QString MIhmLanguages::getBtnLabelIdentification(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_IDENTIF, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getBtnLabelEndIdentification(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_END_IDENTIF, eTranslType, sResult);
	return sResult;
}


QString MIhmLanguages::getBtnLabelTakeControl(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_TAKE_CTRL, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getBtnLabelReturnControl(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_BTN_RETURN_CTRL, eTranslType, sResult);
	return sResult;
}


QString MIhmLanguages::getInControlTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_IN_CONTROL, eTranslType, sResult);
	return sResult;
}

QString MIhmLanguages::getNotInControlTranslation(enumTranslTarget eTranslType)
{
	QString sResult;
	getLabelTranslation(IHM_LABEL_NOT_IN_CONTROL, eTranslType, sResult);
	return sResult;
}


//-----------------------------------------------------------


LanguageList::LanguageList()
{

}

LanguageList::~LanguageList()
{
	while(!m_plstLanguage->isEmpty())
		delete m_plstLanguage->takeLast();

	delete m_plstLanguage;

}



bool LanguageList::lessThan(const LanguageLabel* x1, const LanguageLabel* x2) 
{ 
	return QString::compare(x1->m_sLabelName, x2->m_sLabelName) > 0; 
}


void LanguageList::sort()
{
	//qSort(m_plstLanguage->begin(),m_plstLanguage->end(), LanguageList::lessThan);

	std::sort(m_plstLanguage->begin(), m_plstLanguage->end(), LanguageList::lessThan);
}

bool LanguageList::getTranslation(QString sLabelName, MIhmLanguages::enumTranslTarget eTranslType, QString &sResult)
{
	LanguageLabel *pFound;
	bool bFound = false;

	LanguageLabel oWanted;
	oWanted.m_sLabelName = sLabelName;
	
	QList<LanguageLabel*>::iterator it;
	//it = qBinaryFind(m_plstLanguage->begin(), m_plstLanguage->end(), &oWanted, LanguageList::lessThan);
	it = std::lower_bound(m_plstLanguage->begin(), m_plstLanguage->end(), &oWanted, LanguageList::lessThan);

	if( it !=  m_plstLanguage->end())
	{
		pFound = *it;
		bFound = true;
	}
	
	if(bFound)
	{

		if(eTranslType == MIhmLanguages::enuTranslTargetDesktop)
		{
			sResult = pFound->m_sLabelTranslationForDsk;
		}
		else if(eTranslType == MIhmLanguages::enuTranslTargetWeb)
		{
			sResult = pFound->m_sLabelTranslationForWeb;
		}
		else
			sResult = pFound->m_sLabelTranslation;

	}

	return bFound;
}



bool LanguageList::findLanguageLabel(QString sLabelName, LanguageLabel ** pFound)
{
	LanguageLabel oWanted;
	oWanted.m_sLabelName = sLabelName;
	
	QList<LanguageLabel*>::iterator it;
	
	//it = qBinaryFind(m_plstLanguage->begin(), m_plstLanguage->end(), &oWanted, LanguageList::lessThan);
	it = std::lower_bound(m_plstLanguage->begin(), m_plstLanguage->end(), &oWanted, LanguageList::lessThan);

	if( it !=  m_plstLanguage->end())
	{
		*pFound = *it;
		return true;
	}

	return false; //NULL
}


