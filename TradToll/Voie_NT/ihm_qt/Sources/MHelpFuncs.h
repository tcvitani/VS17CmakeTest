

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
	static QString escapeString( QString str );
	static QString escapeRegex( QString str );
	static QString unescapeString( const QByteArray& ba, bool* ok );
	static QString replaceNewLinesForDsk(QString str);
	static QString replaceNewLinesForWeb(QString str);

	static QString getStringItem(const QString & sData, int index, const QString & sSeparator, bool bTrim);
	static bool isInt(const QString &s);
	static QString loadFileContent(QString sFilePath);
	static QString getAttributeText(QDomNode *pNode, QString sAttName, bool bTrim = false);
	static bool setFileCSSToWidget(QString sFilePath, QWidget * pWidget);
	
	//static int calcLuhnKey(QString sData);
	static QString GenerateCheckCharacter(QString sInput, int n);
	static QString GenerateCheckCharacter_M3(QString sInput, int n);
	static bool checkLuhnKey(QString sValue);
	static bool checkLuhnKeyMod16(QString sValue);
	static bool checkLuhnKeyMod16_M3(QString sValue);

	static QString getVideoCmdString(int iCmdId);
	static int getObjectIdFromString(QString sObjectName);
	static QString getStringFromObjectId(int iObjectId);
	static int convertAlignment(QString sAlign);

	static QString deepCopy(const QString &right); 

	static QRect convertStrToRect(const QString& sGeometry);

	static QString cleanAbsolutePath(QString sPath);
private:
	static bool ishexnstring(const QString& string);

	static int CodePointFromCharacter(QChar ch); 
	static QString CharacterFromCodePoint(int codePoint);

	static int RoundHalfToEven(double dNum);

};


#endif
