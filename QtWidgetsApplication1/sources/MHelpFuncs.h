

#ifndef MHelperFuncs_H
#define MHelperFuncs_H

#include <QString>
#include <QByteArray>
#include <QRect>



#define IHM_STRNCPY(dst,src,len) (strncpy(dst,src,len)[(len)-1]='\0')
#define IHM_STRNCAT(dst,src,len) (strncat(dst,src,len)[(len)-1]='\0')

#define STR_NEWLINE_FOR_WEB "<BR>"
#define STR_NEWLINE_FOR_DSK "\n"

class QDomNode;
class QWidget;


class MHelpFuncs 
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

	static QString getStringItem(const QString & sData, int index, const QString & sSeparator, bool bTrim);
	static bool isInt(const QString &s);
	static QString loadFileContent(QString sFilePath);
	static QString escapeStringForSprintf(const QString & sData);

private:
	static bool ishexnstring(const QString& string);
};


#endif
