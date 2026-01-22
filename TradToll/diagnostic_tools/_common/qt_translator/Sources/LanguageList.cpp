/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 QT_translator													 */
/* FILE:	 LanguageList.cpp												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include "LanguageList.h"

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*---------------------------------- CODE: ----------------------------------*/

LanguageList::LanguageList(QString languageName)
{
	m_sLanguageID = languageName;
	m_plstLanguage = new QList<LanguageLabel*>();
}

LanguageList::~LanguageList()
{
	while(!m_plstLanguage->isEmpty())
		delete m_plstLanguage->takeLast();
	
	delete m_plstLanguage;
	
}

void LanguageList::append(LanguageLabel* lbl)
{
	m_plstLanguage->append(lbl);
}

bool LanguageList::lessThan(const LanguageLabel* x1, const LanguageLabel* x2) 
{ 
	return *x1 < *x2; 
}


void LanguageList::sort()
{
	//qSort(m_plstLanguage->begin(), m_plstLanguage->end(), LanguageList::lessThan);
	std::sort(m_plstLanguage->begin(), m_plstLanguage->end(), LanguageList::lessThan);
}

bool LanguageList::getTranslation(QString sLabelName, QString &sResult)
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

/*-------------------------------- END OF FILE ------------------------------*/
