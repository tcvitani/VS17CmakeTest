/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 QT_translator													 */
/* FILE:     MTranslator.h													 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MTRANSLATOR_H
#define MTRANSLATOR_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QString>
#include <QTranslator>

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#if defined MTRANS_DLLEXPORT
#define MTRANS_DLLSPEC  Q_DECL_EXPORT
#else
#define MTRANS_DLLSPEC Q_DECL_IMPORT
#endif

#define MTRANS_SUCCESS 0
#define MTRANS_FAIL 1
#define MTRANS_NO_REQ_LANGUAGE 2

/*-------------------------------- TYPEDEFS:  -------------------------------*/

// forward declaration
class TranslatorImpl;

class MTRANS_DLLSPEC MTranslator : public QTranslator
{
	Q_OBJECT
public:
	MTranslator(QObject *parent = 0);
	virtual ~MTranslator();

	int initLanguagesFromFile(QString langFilePath, QString language);

	QString getInitErrorString();

	bool setLanguage(QString sLanguageID);
	bool setLanguage(int index);
	int getCurrentLangIndex();

	const QStringList& getLangIDsList();
	const QStringList& getLangNativeNamesList();

	QString getLabelTranslation(QString label);
	
	QString language() const;

	// overriden from QTranslator
	bool isEmpty () const;

	// overriden from QTranslator
	QString translate(const char * context, const char * sourceText, const char * disambiguation, int n = -1) const;

	QString filePath() const;

	bool load(const QString& filename,
		const QString& directory = QString(),
		const QString& search_delimiters = QString(),
		const QString& suffix = QString()) {
		return true;
	};

	bool load(const QLocale& locale,
		const QString& filename,
		const QString& prefix = QString(),
		const QString& directory = QString(),
		const QString& suffix = QString()) {
		return true;
	};

	bool load(const uchar* data, int len,
		const QString& directory = QString()) {
		return true;
	};


private:
	TranslatorImpl* m_pTransImpl;
	QString m_langFilePath;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/


