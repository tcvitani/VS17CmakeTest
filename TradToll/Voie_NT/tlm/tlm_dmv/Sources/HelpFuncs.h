/****************************************************************************
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** Licensees holding a valid Qt License Agreement may use this file in
** accordance with the rights, responsibilities and obligations
** contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of this licensing
** agreement are not clear to you.
**
** Further information about Qt licensing is available at:
** http://www.trolltech.com/products/qt/licensing.html or by
** contacting info@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef HelperFuncs_H
#define HelperFuncs_H

#include <QString>
#include <QByteArray>
class QDomNode;
class QDomNodeList;

class HelpFuncs 
{

public:
	static QString ByteArrayToHexStr(const QByteArray & baData);
	static QByteArray HexStrToByteArray(const QString & sData);
	static QString RemoveSpacesFromHexStr(const QString & sData);
	static QString AddSpacesToHexStr(const QString & sData);
	static QString HexStrToBinStr(const QString & sData);
	static QByteArray ByteArrayToHexArray(const QByteArray & baData);
	static QByteArray HexArrayToByteArray(const QByteArray & sData);
	static QString ByteArrayToAnsi(const QByteArray & baData);
	static QString ByteArrayToVisibleAnsi(const QByteArray & baData);
	static QByteArray StrToByteArray(const QString & sData);

	static QString loadFileContent(QString sFilePath);

	static QString getAttributeText(QDomNode *pNode, QString sAttName, bool bTrim = false);
	static QString getNodeText(QDomNode *pNode, bool bTrim);
	static QString getElementText(QDomNode *pNode, QString sElName, bool bTrim = false, QString sNS = QString());
	static QString getSubElementText(QDomNode *pNode, QString sElPath, QString sElName, bool bTrim, QString sNS = QString());
	static bool getSubNodeList(QDomNode *pNode, QString sElPath, QString sLastElName, QDomNodeList * pResultNodeList, QString sNS = QString());
	static bool getNodeWhereAttribute(QDomNodeList *pNodeList, 
		QString sConditionAttributeName, QString sConditionAttributeValue, QDomNode *pResultNode);
	static QString getElementAttributeText(QDomNode *pNode, QString sElName, QString sAttributeName, bool bTrim = false, QString sNS = QString());

	static QString escapeStringForSprintf(const QString & sData);
	static bool isElementPresent(QDomNode *pNode, QString sElName);

};


#endif
