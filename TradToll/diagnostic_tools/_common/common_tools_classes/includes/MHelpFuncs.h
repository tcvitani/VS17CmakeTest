/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:     MHelpFuncs.h													 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef COMMON_TOOLS_HELPER_FUNCS_H
#define COMMON_TOOLS_HELPER_FUNCS_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QString>
#include <QByteArray>
#include <QRect>

/*-------------------------------- RESERVED:  -------------------------------*/



/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/
#if defined COMMON_TOOLS_CLASSES_EXPORT
	#define COMMON_TOOLS_CLASSES_DLLSPEC  Q_DECL_EXPORT
#else
	#define COMMON_TOOLS_CLASSES_DLLSPEC Q_DECL_IMPORT
#endif

/*-------------------------------- TYPEDEFS:  -------------------------------*/

class QDomNode;
class QWidget;

class COMMON_TOOLS_CLASSES_DLLSPEC MHelpFuncs 
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
		static QString deepCopy(const QString &right); 
		static QRect convertStrToRect(const QString& sGeometry);
		static QString getProcessFileName();
		static QString getVersionInfo();
		static bool appendUnsignedCharStringToQString(unsigned char *uscpSource, size_t srcSize, QString *ucspDestination);
		static errno_t QStringToUnsignedCharString(QString qsSource, unsigned char *ucspDestination, size_t destSize, bool doConversionWithMemcpy);

	private:
		static bool ishexnstring(const QString& string);
};

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
