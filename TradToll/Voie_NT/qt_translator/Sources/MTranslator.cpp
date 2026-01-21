/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 QT_translator													 */
/* FILE:	 MTranslator.cpp												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include "MTranslator.h"

#include "TranslatorImpl.h"

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*---------------------------------- CODE: ----------------------------------*/

MTranslator::MTranslator(QObject *parent):
QTranslator(parent),
m_pTransImpl(NULL)
{
	m_pTransImpl = new TranslatorImpl();
	
}

MTranslator::~MTranslator()
{
	delete m_pTransImpl;
	m_pTransImpl = NULL;
}

int MTranslator::initLanguagesFromFile(QString langFilePath, QString language)
{
	m_langFilePath = langFilePath;
	return m_pTransImpl->initLanguagesFromFile(langFilePath, language, NULL);
}

QString MTranslator::getInitErrorString()
{
	return m_pTransImpl->getInitErrorString();
}

bool MTranslator::setLanguage(QString sLanguageID)
{
	return m_pTransImpl->setLanguage(sLanguageID);
}

bool MTranslator::setLanguage(int index)
{
	return m_pTransImpl->setLanguage(index);
}

// void MTranslator::setSystemEncoding(QString encoding)
// {
// 	m_pTransImpl->setSystemEncoding(encoding);
// }

int MTranslator::getCurrentLangIndex()
{
	return m_pTransImpl->getCurrentLangIndex();
}

const QStringList& MTranslator::getLangIDsList()
{
	return m_pTransImpl->getLangIDsList();
}

const QStringList& MTranslator::getLangNativeNamesList()
{
	return m_pTransImpl->getLangNativeNamesList();
}

QString MTranslator::getLabelTranslation(QString label)
{
	return m_pTransImpl->getLabelTranslation(label);
}

bool MTranslator::isEmpty () const
{
	return m_pTransImpl->isEmpty();
}

QString MTranslator::translate(const char * context, const char * sourceText, const char * disambiguation, int n) const
{
	return m_pTransImpl->translate(context, sourceText, disambiguation);
}

QString MTranslator::language() const
{
	return m_pTransImpl->language();
}

QString MTranslator::filePath() const
{
	return m_langFilePath;
}