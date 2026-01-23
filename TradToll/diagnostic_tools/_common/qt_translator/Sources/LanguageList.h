/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 QT_translator														 */
/* FILE:     LanguageList.h														 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef LANGUAGE_LIST_H
#define LANGUAGE_LIST_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QString>
#include <QList>

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/

class LanguageLabel
{
public:
	bool operator<(const LanguageLabel& x) const { return (QString::compare(m_sLabelName, x.m_sLabelName,Qt::CaseSensitive)<0); }
	
	QString m_sLabelName;
	QString m_sLabelTranslation;
};


class LanguageList
{
public:
	LanguageList(QString languageName);
	~LanguageList();
	
	inline const QString& getLanguageID() { return m_sLanguageID; }
	
	static bool lessThan(const LanguageLabel* x1, const LanguageLabel* x2); 
	
	bool getTranslation(QString sLabelName, QString &sResult);
	bool findLanguageLabel(QString sLabelName, LanguageLabel ** pFound);
	
	void sort();

	void append(LanguageLabel* lbl);

private:
	QList<LanguageLabel*> *m_plstLanguage;
	QString m_sLanguageID;

};


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/


