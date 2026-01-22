/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:	 MHelpFuncs.cpp													 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QApplication>
#include <QStringList>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>


#include "MHelpFuncs.h"

#include "windows.h"
#include "winver.h"

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


#define IHM_STRNCPY(dst,src,len) (strncpy(dst,src,len)[(len)-1]='\0')
#define IHM_STRNCAT(dst,src,len) (strncat(dst,src,len)[(len)-1]='\0')

#define STR_NEWLINE_FOR_WEB "<BR>"
#define STR_NEWLINE_FOR_DSK "\n"

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*---------------------------------- CODE: ----------------------------------*/

QByteArray MHelpFuncs::StrToByteArray(const QString & sData)
{
	QByteArray baRetVal;
	int iLen,i;
	QChar cByteVal;
	QByteArray baTemp;
	
	iLen = sData.length();

	for(i=0; i < iLen; i++)
	{
		cByteVal = sData.at(i);
		
		if(cByteVal.toLatin1()>=32)
			baTemp = QByteArray(1,cByteVal.toLatin1()); 
			
		baRetVal += baTemp;
	}

	return baRetVal;	
}

QString MHelpFuncs::ByteArrayToAnsi(const QByteArray & baData)
{
	QString sRetVal;
	int iLen,i;
	char cByteVal;
	QString sTemp;
	
	iLen = baData.size();

	for(i=0; i < iLen; i++)
	{
		cByteVal = baData[i];
		
		if(cByteVal>=32)
			sTemp = QString("%1").arg(cByteVal); 
		else
		{
			sTemp = QString("%1").arg((int)cByteVal,(int)1, (int)16); 
			sTemp = sTemp.rightJustified(2,'0',true);
			sTemp = QString("#%1").arg(sTemp);
		}
		
		sRetVal += sTemp;
	}

	return sRetVal;

}

QString MHelpFuncs::ByteArrayToVisibleAnsi(const QByteArray & baData)
{
	QString sRetVal;
	int iLen,i;
	char cByteVal;
	QString sTemp;
	
	iLen = baData.size();

	for(i=0; i < iLen; i++)
	{
		cByteVal = baData[i];
		
		if(cByteVal>=32)
			sTemp = QString("%1").arg(cByteVal); 
		
		sRetVal += sTemp;
	}

	return sRetVal;

}

QString MHelpFuncs::HexStrToBinStr(const QString & sData)
{
	QString sRetVal;
	char cByteVal;
	int iLen, i;
	QString sTemp;
	bool bOk;
	QString sCopy = RemoveSpacesFromHexStr(sData);

	iLen = sCopy.length();

	for(i=0; i < iLen - 1; i++)
	{
		if(i%2 == 0)
		{
			sTemp = sCopy.at(i);
			sTemp += sCopy.at(i+1);

			cByteVal = sTemp.toInt(&bOk,(int)16);
			
			if(bOk)
			{
				sRetVal += cByteVal;
			}
		}

	}

	return sRetVal;
}

QString MHelpFuncs::ByteArrayToHexStr(const QByteArray & baData)
{
	QString sRetVal;
	int iLen,i;
	uchar cByteVal;
	QString sTemp;
	
	iLen = baData.size();

	for(i=0; i < iLen; i++)
	{
		cByteVal = baData[i];
		sTemp = QString("%1").arg((uint)cByteVal,(int)1, (int)16 );
		sTemp = sTemp.rightJustified(2,'0',true);
		sRetVal += sTemp;
	}

	return sRetVal;
}

QByteArray MHelpFuncs::HexStrToByteArray(const QString & sData)
{
	QByteArray baRetVal;
	char cByteVal;
	int iLen, i;
	QString sTemp;
	bool bOk;
	QString sCopy = RemoveSpacesFromHexStr(sData);

	iLen = sCopy.length();

	for(i=0; i < iLen - 1; i++)
	{
		if(i%2 == 0)
		{
			sTemp = sCopy.at(i);
			sTemp += sCopy.at(i+1);

			cByteVal = sTemp.toInt(&bOk,(int)16);
			
			if(bOk)
			{
				baRetVal.append(cByteVal);
			}
		}

	}

	return baRetVal;
}

QString MHelpFuncs::RemoveSpacesFromHexStr(const QString & sData)
{
	QString sRezult;
	int iLen, i;

	iLen = sData.length();

	for(i=0; i < iLen; i++)
	{

		if(!sData.at(i).isSpace())
		{
			sRezult += sData.at(i);
		}
	}

	return sRezult;

}

QString MHelpFuncs::AddSpacesToHexStr(const QString & sData)
{
	QString sRezult;

	int iLen, i;

	QString sDataCpy = sData.trimmed();
	iLen = sDataCpy.length();

	for(i=0; i < iLen; i++)
	{
		if(i%2 == 1)
		{
			sRezult += sDataCpy.at(i);
			sRezult += " ";
		}
		else
			sRezult += sDataCpy.at(i);

		
	}

	return sRezult;
}

QByteArray MHelpFuncs::ByteArrayToHexArray(const QByteArray & baData)
{
	QByteArray baRetVal;
	QByteArray baTemp;
	int iLen,i;
	unsigned char cByteVal;
	
	iLen = baData.size();

	for(i=0; i < iLen; i++)
	{
		cByteVal = baData[i];
		baTemp = QByteArray::number(cByteVal, 16).toUpper();
		baTemp = baTemp.rightJustified(2,'0',true);
		baRetVal.append(baTemp);  
	}

	return baRetVal;
}

QByteArray MHelpFuncs::HexArrayToByteArray(const QByteArray & baData)
{
	QByteArray baRetVal;
	unsigned char cByteVal;
	int iLen, i;
	QByteArray baTemp;
	uint uiNum;
	bool bOK;

	iLen = baData.size();

	for(i=0; i < iLen - 1; i++)
	{
		if(i%2 == 0)
		{
			baTemp = baData.mid(i,2);
			uiNum = baTemp.toUInt(&bOK,16);
			
			if(bOK)
				cByteVal = (unsigned char)uiNum;
			else
				cByteVal = 0;

			baRetVal.append(cByteVal);
		}

	}

	return baRetVal;
}

bool MHelpFuncs::ishexnstring(const QString& string) 
{
  for (int i = 0; i < string.length(); i++) 
  {
    if (isxdigit(string[i] == 0))
      return false;
  }

  return true;
} 

QString MHelpFuncs::unescapeString( const QByteArray& ba, bool* ok ) 
{
  Q_ASSERT( ok );
  *ok = false;
  QString res;
  QByteArray seg;
  bool bs = false;
  for ( int i = 0, size = ba.size(); i < size; ++i ) 
  {
    const char ch = ba[i];

    if ( !bs ) 
	{
      if ( ch == '\\' )
        bs = true;
      else
        seg += ch;
    } 
	else 
	{
      bs = false;
      switch ( ch ) 
	  {
        case 'b':
          seg += '\b';
          break;

        case 'f':
          seg += '\f';
          break;

        case 'n':
          seg += '\n';
          break;

        case 'r':
          seg += '\r';
          break;

        case 't':
          seg += '\t';
          break;

        case 'u':
        {
          res += QString::fromUtf8( seg );
          seg.clear();

          if ( i > size - 5 ) 
		  {
            //error
            return QString();
          }

          const QString hex_digit1 = QString::fromUtf8( ba.mid( i + 1, 2 ) );
          const QString hex_digit2 = QString::fromUtf8( ba.mid( i + 3, 2 ) );
          i += 4;

          if ( !ishexnstring( hex_digit1 ) || !ishexnstring( hex_digit2 ) ) 
		  {
            //TRACE_W(QString("Not an hex string:%1 %2").arg(hex_digit1).arg(hex_digit2));
            return QString();
          }

          bool hexOk;
          const ushort hex_code1 = hex_digit1.toShort( &hexOk, 16 );

          if (!hexOk) 
		  {
            //TRACE_W(QString("error converting hex value to short::%1").arg(hex_digit1));
            return QString();
          }

          const ushort hex_code2 = hex_digit2.toShort( &hexOk, 16 );

          if (!hexOk) 
		  {
            //TRACE_W(QString("error converting hex value to short::%1").arg(hex_digit2));
            return QString();
          }

          res += QChar(hex_code2, hex_code1);
          break;
        }

        case '\\':
          seg  += '\\';
          break;

        default:
          seg += ch;
          break;
      }
    }
  }

  res += QString::fromUtf8( seg );
  *ok = true;

  return res;
}

QString MHelpFuncs::escapeString( QString str )
{
  str.replace( QLatin1String( "\\" ), QLatin1String( "\\\\" ) );
  str.replace( QLatin1String( "\"" ), QLatin1String( "\\\"" ) );
  str.replace( QLatin1String( "\b" ), QLatin1String( "\\b" ) );
  str.replace( QLatin1String( "\f" ), QLatin1String( "\\f" ) );
  str.replace( QLatin1String( "\n" ), QLatin1String( "\\n" ) );
  str.replace( QLatin1String( "\r" ), QLatin1String( "\\r" ) );
  str.replace( QLatin1String( "\t" ), QLatin1String( "\\t" ) );
  
  return QString( QLatin1String( "%1" ) ).arg( str );
} 

QString MHelpFuncs::escapeRegex( QString str )
{
  str.replace( QLatin1String( "\\" ), QLatin1String( "\\\\" ) );
 
  return QString("%1").arg( str );
}

QString MHelpFuncs::replaceNewLinesForDsk(QString str)
{
	return str.replace('\xF5',STR_NEWLINE_FOR_DSK);
}

QString MHelpFuncs::replaceNewLinesForWeb(QString str)
{
	return str.replace("&sect;",STR_NEWLINE_FOR_WEB);
}

QString MHelpFuncs::getStringItem(const QString & sData, 
								  int index, 
								  const QString & sSeparator, 
								  bool bTrim)
{
	QString sRes;
	QStringList sLst =  sData.split(sSeparator);

	if(sLst.size()>=index+1)
	{	sRes = sLst.at(index);

		if(bTrim)
			sRes = sRes.trimmed();
	}
	return sRes;
}

bool MHelpFuncs::isInt(const QString &s)
{
	bool bRet;

	s.toInt(&bRet, 10);

	return bRet;
}

QString MHelpFuncs::loadFileContent(QString sFilePath)
{
	QString sRes;
	QFile file(sFilePath);

	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream txtStream;
		txtStream.setDevice(&file);
		txtStream.setEncoding(QStringConverter::Utf8);

		sRes = txtStream.readAll(); 
	}

	return sRes;
}

bool MHelpFuncs::setFileCSSToWidget(QString sFilePath, QWidget * pWidget)
{
	QString sFileContent = loadFileContent(sFilePath);

	if(sFileContent!="" && pWidget!=NULL)
	{
		pWidget->setStyleSheet(sFileContent);
		return true;
	}
	
	return false;
}

QString MHelpFuncs::deepCopy(const QString &right)
{ 
	return QString::fromUtf16(right.utf16());
}

QString MHelpFuncs::getProcessFileName()
{
	QString sName = qApp->applicationFilePath();
	QFileInfo sInfo(sName);
	sName = sInfo.fileName();
	sName = sName.replace(QString(".%1").arg(sInfo.suffix()), "");

	return sName;

}


QString MHelpFuncs::getVersionInfo()
{
	TCHAR moduleName[MAX_PATH+1];
	DWORD versionSize = GetModuleFileName(0, moduleName, MAX_PATH);	

	DWORD dummyZero;
	versionSize = GetFileVersionInfoSize(moduleName, &dummyZero);
	if(versionSize == 0)
	{
		return NULL;
	}
	void* pVersion = malloc(versionSize);
	if(pVersion == NULL)
	{
		return NULL;
	}
	if(!GetFileVersionInfo(moduleName, NULL, versionSize, pVersion))
	{
		free(pVersion);
		return NULL;
	}

	UINT length;
	VS_FIXEDFILEINFO* pFixInfo;
	VerQueryValue(pVersion, (LPCWSTR)QString("\\").utf16(), (LPVOID*)&pFixInfo, &length);

	QString sResult;
	
	sResult = QString("%1.%2.%3.%4").arg((pFixInfo->dwFileVersionMS&0xFFFF0000)>>16).arg((pFixInfo->dwFileVersionMS&0x0000FFFF))
			.arg((pFixInfo->dwFileVersionLS&0xFFFF0000)>>16).arg(pFixInfo->dwFileVersionLS&0x0000FFFF);

	free(pVersion);

	return sResult;
}

bool MHelpFuncs::appendUnsignedCharStringToQString(unsigned char *uscpSource, size_t srcSize, QString *ucspDestination)
{
	int				i = 0;
	unsigned char	*pTemp = uscpSource;
	size_t			sizetSrcNumberOfCharacters = (srcSize/sizeof(uscpSource[0]));

	for (i = 0; (*pTemp != '\0' && i < sizetSrcNumberOfCharacters); i++)
	{
		ucspDestination->append(QChar(*pTemp));
		++pTemp;
	}

	return TRUE;
}

errno_t MHelpFuncs::QStringToUnsignedCharString(QString qsSource, unsigned char *ucspDestination, size_t destSize, bool doConversionWithMemcpy)
{
	unsigned char	*pTemp = ucspDestination;
	size_t			sizetDestNumberOfCharacters = (destSize/sizeof(ucspDestination[0])) - 1; // -1 because last character is reserved for '\0'
		
	if (!doConversionWithMemcpy)
	{
		for (int i = 0; i < qsSource.size(); i++)
		{
			if (i >= sizetDestNumberOfCharacters)
			{
				*pTemp = '\0';
				return ERANGE;
			}

			*pTemp = qsSource.at(i).unicode();
			++pTemp;
		}

		// Append null char at the end of string
		*pTemp = '\0';
	}
	else
	{
		if (qsSource.size() > sizetDestNumberOfCharacters)
			return ERANGE;

		return memcpy_s(ucspDestination, destSize, qsSource.toLatin1().data(), ((qsSource.size() + 1) * sizeof(unsigned char)));			
	}
	return 0;
}
/*-------------------------------- END OF FILE ------------------------------*/
