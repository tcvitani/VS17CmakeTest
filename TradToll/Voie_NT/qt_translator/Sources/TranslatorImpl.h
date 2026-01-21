/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 QT_translator														 */
/* FILE:     TranslatorImpl.h														 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef TRANS_IMPL_H
#define TRANS_IMPL_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QString>
#include <QTranslator>
#include <QDomNode>
#include <QStringList>
#include "LanguageList.h"

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/

class TranslatorImpl
{
public:
	enum InitConstants {IMPL_SUCCESS = 0, IMPL_FAIL = 1, IMPL_NO_REQ_LANGUAGE = 2};

	TranslatorImpl();
	virtual ~TranslatorImpl();

	TranslatorImpl::InitConstants initLanguagesFromFile(QString langFilePath, QString language, QString systemEncoding);

	inline QString getInitErrorString()
	{
		return m_sInitErrorString;
	}

	bool setLanguage(QString sLanguage);
	bool setLanguage(int index);

	int getCurrentLangIndex();

	void setSystemEncoding(QString encoding);
	QString getLabelTranslation(QString label) const;

	const QStringList& getLangIDsList()
	{
		return m_lstLangIDs;
	}

	const QStringList& getLangNativeNamesList()
	{
		return m_lstLangNativeNames;
	}

	bool isEmpty () const
	{
		return !isInitialized;
	}

	QString translate(const char * context, const char * sourceText, const char * disambiguation) const;

	QString language() { return m_sSelectedLanguage; };

private:
	bool isInitialized;
	QString m_sInitErrorString;

	QString m_sSelectedLanguage;
	
	QList<LanguageList*> m_lstAllLanguages;
	
	LanguageList * m_plstSelectedLanguage;

	static QString getAttributeText(QDomNode *pNode, QString sAttName, bool bTrim = false);
	static QString replaceNewLinesForDsk(QString str);

	QStringList m_lstLangNativeNames;
	QStringList m_lstLangIDs;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
