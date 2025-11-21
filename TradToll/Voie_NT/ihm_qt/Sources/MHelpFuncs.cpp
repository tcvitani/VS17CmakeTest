
#include <QStringList>
#include <QWidget>
#include <QDomNode>
#include <QFile>
#include <QTextStream>
#include <Math.h>
#include <QDir>

#include "MHelpFuncs.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}

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

	iLen = sData.length();

	for(i=0; i < iLen; i++)
	{
		if(i%2 == 0)
		{
			sRezult += sData.at(i);
			sRezult += " ";
		}
		else
			sRezult += sData.at(i);

		
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


bool MHelpFuncs::ishexnstring(const QString& string) {
  for (int i = 0; i < string.length(); i++) {
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
  for ( int i = 0, size = ba.size(); i < size; ++i ) {
    const char ch = ba[i];
    if ( !bs ) {
      if ( ch == '\\' )
        bs = true;
      else
        seg += ch;
    } else {
      bs = false;
      switch ( ch ) {
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

          if ( i > size - 5 ) {
            //error
            return QString();
          }

          const QString hex_digit1 = QString::fromUtf8( ba.mid( i + 1, 2 ) );
          const QString hex_digit2 = QString::fromUtf8( ba.mid( i + 3, 2 ) );
          i += 4;

          if ( !ishexnstring( hex_digit1 ) || !ishexnstring( hex_digit2 ) ) {
            TRACE_WEB_W(QString("Not an hex string:%1 %2").arg(hex_digit1).arg(hex_digit2));
            return QString();
          }
          bool hexOk;
          const ushort hex_code1 = hex_digit1.toShort( &hexOk, 16 );
          if (!hexOk) {
            TRACE_WEB_W(QString("error converting hex value to short::%1").arg(hex_digit1));
            return QString();
          }
          const ushort hex_code2 = hex_digit2.toShort( &hexOk, 16 );
          if (!hexOk) {
            TRACE_WEB_W(QString("error converting hex value to short::%1").arg(hex_digit2));
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
	return str.replace(IHM_STR_NEW_LINE,STR_NEWLINE_FOR_DSK);
}


QString MHelpFuncs::replaceNewLinesForWeb(QString str)
{
	return str.replace("&sect;",STR_NEWLINE_FOR_WEB);
}




QString MHelpFuncs::getStringItem(const QString & sData, int index, const QString & sSeparator, bool bTrim)
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


QString MHelpFuncs::getAttributeText(QDomNode *pNode, QString sAttName, bool bTrim)
{
	QDomNamedNodeMap mapAttributes = pNode->attributes();
	QString sRetValue;

	for(int i=0;i<mapAttributes.count();i++)
	{
		//to make it case insensitive for attribute names...
		if(!mapAttributes.item(i).isNull())
			if(QString::compare(sAttName, mapAttributes.item(i).nodeName(),Qt::CaseInsensitive)==0)
			{
				if(bTrim)
					sRetValue = mapAttributes.item(i).nodeValue().trimmed();
				else
					sRetValue = mapAttributes.item(i).nodeValue();

				break;
			}
	}
	
	return sRetValue;
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



// unsigned char LuhnKey(unsigned char *p_str, unsigned char length)
// {
// 	unsigned char i, key = 0;
// 	unsigned long doubled_value, doubled_values_sum = 0;
// 
// 	i = length;
// 
// 	while (i > 0)
// 	{
// 		doubled_value = ((p_str[(i - 1)] & 0x0F) << 1);
// 
// 		if (doubled_value >= 10)
// 				doubled_value -= 9;
// 
// 		doubled_values_sum += doubled_value;
// 
// 		if (--i > 0)
// 			doubled_values_sum += p_str[(i-- - 1)];
// 	}
// 
// 	key = (unsigned char) (10 - (doubled_values_sum % 10));
// 	
// 	if (key == 10)
// 			key = 0;
// 
// 	return (key);
// }


// int MHelpFuncs::calcLuhnKey(QString sData)
// {
// 	int i, key = 0;
// 	unsigned long doubled_value, doubled_values_sum = 0;
// 
// 	int iSize = sData.size();
// 	unsigned char ucCurrentDigit;
// 	unsigned char ucNewDigit;
// 		
// 	i = iSize; //ajde na zadnjega
// 
// 	while (i > 0) 
// 	{
// 		if(sData.at(i-1).isDigit()) //uzmi trenutni znak
// 			ucCurrentDigit = (unsigned char)sData.at(i-1).digitValue(); //ako je broj uzmi koja je to znamenka 0-9 (int)
// 		else
// 			return -1;
// 
// 		doubled_value = ((ucCurrentDigit & 0x0F) << 1); //uzmi donjih 8 bitova i pomnozi to sa 2
// 
// 		if (doubled_value >= 10) 
// 				doubled_value -= 9;
// 
// 		doubled_values_sum += doubled_value;
// 
// 
// 		if (--i > 0) 
// 		{	
// 			if(sData.at(i-1).isDigit())
// 				ucNewDigit = (unsigned char)sData.at(i-1).digitValue();
// 			else
// 				return -1;
// 
// 			doubled_values_sum += ucNewDigit;
// 			i--;
// 		}
// 	}
// 
// 	key = (unsigned char) (10 - (doubled_values_sum % 10));
// 	
// 	if (key == 10)
// 			key = 0;
// 
// 	return (key);
// }
// 

int MHelpFuncs::CodePointFromCharacter(QChar ch) 
{ 
	QString s;
	s.append(ch);
	
	bool ok;
	int hex = s.toInt(&ok, 16);


	if(ok)
	{
			return hex;
	}
	else
		return -1;
}

QString MHelpFuncs::CharacterFromCodePoint(int codePoint) 
{ 
	QString s = QString::number( codePoint, 16);
	return s;
}
 
//The function to generate a check character is:
// n = NumberOfValidInputCharacters (10 -decimal or 16 -hexadecimal)
QString MHelpFuncs::GenerateCheckCharacter(QString sInput, int n) 
{
 
	int factor = 2;
	int sum = 0;
 
	// Starting from the right and working leftwards is easier since 
	// the initial "factor" will always be "2" 
	for (int i = sInput.size() - 1; i >= 0; i--) {
		int codePoint = MHelpFuncs::CodePointFromCharacter(sInput[i]);
		if(codePoint<0) //error converting char
			return "";

		int addend = factor * codePoint;
 
		// Alternate the "factor" that each "codePoint" is multiplied by
		factor = (factor == 2) ? 1 : 2;
 
		// Sum the digits of the "addend" as expressed in base "n"
		addend = ((double)addend / n) + (addend % n); //compiler always truncate decimals 
		sum += addend;
	}
 
	// Calculate the number that must be added to the "sum" 
	// to make it divisible by "n"
	int remainder = sum % n;
	int checkCodePoint = (n - remainder) % n;
 
	return MHelpFuncs::CharacterFromCodePoint(checkCodePoint);
}

int MHelpFuncs::RoundHalfToEven(double dNum)
{
	int iResult = dNum;

	if(iResult>=0)
	{
		if(dNum-iResult == 0.5)
		{
			if(iResult%2 == 1)
					iResult = iResult + 1;
		}
		else if(dNum-iResult > 0.5)
			iResult = iResult + 1;
	}
	else
	{
		if(dNum-iResult == -0.5)
		{
			if(iResult%2 == -1)
					iResult = iResult - 1;
		}
		else if(dNum-iResult > -0.5)
			iResult = iResult - 1;
			
	}


	return iResult;
}

QString MHelpFuncs::GenerateCheckCharacter_M3(QString sInput, int n) 
{
 
	int factor = 2;
	int sum = 0;
 
	// Starting from the right and working leftwards is easier since 
	// the initial "factor" will always be "2" 
	for (int i = sInput.size() - 1; i >= 0; i--) {
		int codePoint = MHelpFuncs::CodePointFromCharacter(sInput[i]);
		if(codePoint<0) //error converting char
			return "";

		int addend = factor * codePoint;
 
		// Alternate the "factor" that each "codePoint" is multiplied by
		factor = (factor == 2) ? 1 : 2;
 
		// Sum the digits of the "addend" as expressed in base "n"
		addend = RoundHalfToEven((double)addend / n) + (addend % n); //compiler always truncate decimals 
		sum += addend;
	}
 
	// Calculate the number that must be added to the "sum" 
	// to make it divisible by "n"
	int remainder = sum % n;
	int checkCodePoint = (n - remainder) % n;
 
	return MHelpFuncs::CharacterFromCodePoint(checkCodePoint);
}



bool MHelpFuncs::checkLuhnKey(QString sValue)
{
	QString s = MHelpFuncs::GenerateCheckCharacter(sValue.left(sValue.size()-1),10);
	return (QString::compare(sValue.right(1), s, Qt::CaseInsensitive)==0);
}

bool MHelpFuncs::checkLuhnKeyMod16(QString sValue)
{
	QString s = MHelpFuncs::GenerateCheckCharacter(sValue.left(sValue.size()-1),16);
	return (QString::compare(sValue.right(1), s, Qt::CaseInsensitive)==0);

}


bool MHelpFuncs::checkLuhnKeyMod16_M3(QString sValue)
{
	QString s = MHelpFuncs::GenerateCheckCharacter_M3(sValue.left(sValue.size()-1),16);
	return (QString::compare(sValue.right(1), s, Qt::CaseInsensitive)==0);

}

QString MHelpFuncs::getVideoCmdString(int iCmdId)
{
	switch(iCmdId)
	{
		case IHM_VIDEO_ON:return QString("IHM_VIDEO_ON");
		case IHM_VIDEO_OFF:return QString("IHM_VIDEO_OFF");
		case IHM_VIDEO_FREEZE:return QString("IHM_VIDEO_FREEZE");
		case IHM_VIDEO_UNFREEZE:return QString("IHM_VIDEO_UNFREEZE");
		case IHM_VIDEO_ZOOM_IN:return QString("IHM_VIDEO_ZOOM_IN");
		case IHM_VIDEO_ZOOM_OUT:return QString("IHM_VIDEO_ZOOM_OUT");
		case IHM_VIDEO_ZOOM:return QString("IHM_VIDEO_ZOOM");
		case IHM_VIDEO_SELECT_SRC:return QString("IHM_VIDEO_SELECT_SRC");
		case IHM_VIDEO_SAVE_BMP:return QString("IHM_VIDEO_SAVE_BMP");
		case IHM_VIDEO_SAVE_JPG:return QString("IHM_VIDEO_SAVE_JPG");
		case IHM_VIDEO_SAVE_OK:return QString("IHM_VIDEO_SAVE_OK");
		case IHM_VIDEO_SAVE_NOK:return QString("IHM_VIDEO_SAVE_NOK");
		case IHM_VIDEO_FREEZE_OK:return QString("IHM_VIDEO_FREEZE_OK");
		case IHM_VIDEO_FREEZE_NOK:return QString("IHM_VIDEO_FREEZE_NOK");		
		
		default:
			return "IHM_VIDEO_CMD_UNKNOWN";
			break;
	}
}




QString MHelpFuncs::getStringFromObjectId(int iObjectId)
{
	switch(iObjectId)
	{
	case IHM_OBJ_All_Object:return QString("IHM_OBJ_All_Object");

// Labels
	case IHM_OBJ_PlazaName:			return QString("IHM_OBJ_PlazaName"); 
	case IHM_OBJ_PlazaName1:			return QString("IHM_OBJ_PlazaName1"); 
	case IHM_OBJ_PlazaName2:			return QString("IHM_OBJ_PlazaName2"); 
	case IHM_OBJ_PlazaName3:			return QString("IHM_OBJ_PlazaName3"); 
	case IHM_OBJ_PlazaNameTitle:	return QString("IHM_OBJ_PlazaNameTitle");
	case IHM_OBJ_PlazaNbr:			return QString("IHM_OBJ_PlazaNbr");
	case IHM_OBJ_PlazaNbrTitle:		return QString("IHM_OBJ_PlazaNbrTitle");
	case IHM_OBJ_LaneNbr:			return QString("IHM_OBJ_LaneNbr");
	case IHM_OBJ_LaneNbrTitle:		return QString("IHM_OBJ_LaneNbrTitle");
	case IHM_OBJ_CollectorID:		return QString("IHM_OBJ_CollectorID");
	case IHM_OBJ_CollectorName:		return QString("IHM_OBJ_CollectorName");      

	case IHM_OBJ_TxtSpare1:			return QString("IHM_OBJ_TxtSpare1");		
	case IHM_OBJ_TxtSpare2:			return QString("IHM_OBJ_TxtSpare2");			
	case IHM_OBJ_TxtSpare3:			return QString("IHM_OBJ_TxtSpare3");
	case IHM_OBJ_TxtSpare4:			return QString("IHM_OBJ_TxtSpare4");
	case IHM_OBJ_TxtSpare5:			return QString("IHM_OBJ_TxtSpare5");
	case IHM_OBJ_TxtSpare6:			return QString("IHM_OBJ_TxtSpare6");
	case IHM_OBJ_TxtSpare7:			return QString("IHM_OBJ_TxtSpare7");
	case IHM_OBJ_TxtSpare8:			return QString("IHM_OBJ_TxtSpare8");
	case IHM_OBJ_TxtSpare9:			return QString("IHM_OBJ_TxtSpare9");
	case IHM_OBJ_TxtSpare10:		return QString("IHM_OBJ_TxtSpare10");
	case IHM_OBJ_TxtSpare11:		return QString("IHM_OBJ_TxtSpare11");
	case IHM_OBJ_TxtSpare12:		return QString("IHM_OBJ_TxtSpare12");
	case IHM_OBJ_TxtSpare13:		return QString("IHM_OBJ_TxtSpare13");
	case IHM_OBJ_TxtSpare14:		return QString("IHM_OBJ_TxtSpare14");
	case IHM_OBJ_TxtSpare15:		return QString("IHM_OBJ_TxtSpare15");
	case IHM_OBJ_TxtSpare16:		return QString("IHM_OBJ_TxtSpare16");
	case IHM_OBJ_TxtSpare17:		return QString("IHM_OBJ_TxtSpare17");
	case IHM_OBJ_TxtSpare18:		return QString("IHM_OBJ_TxtSpare18");
	case IHM_OBJ_TxtSpare19:		return QString("IHM_OBJ_TxtSpare19");
	case IHM_OBJ_TxtSpare20:		return QString("IHM_OBJ_TxtSpare20");
	case IHM_OBJ_TxtSpare21:			return QString("IHM_OBJ_TxtSpare21");		
	case IHM_OBJ_TxtSpare22:			return QString("IHM_OBJ_TxtSpare22");			
	case IHM_OBJ_TxtSpare23:			return QString("IHM_OBJ_TxtSpare23");
	case IHM_OBJ_TxtSpare24:			return QString("IHM_OBJ_TxtSpare24");
	case IHM_OBJ_TxtSpare25:			return QString("IHM_OBJ_TxtSpare25");
	case IHM_OBJ_TxtSpare26:			return QString("IHM_OBJ_TxtSpare26");
	case IHM_OBJ_TxtSpare27:			return QString("IHM_OBJ_TxtSpare27");
	case IHM_OBJ_TxtSpare28:			return QString("IHM_OBJ_TxtSpare28");
	case IHM_OBJ_TxtSpare29:			return QString("IHM_OBJ_TxtSpare29");
	case IHM_OBJ_TxtSpare30:			return QString("IHM_OBJ_TxtSpare30");
	case IHM_OBJ_TxtSpare31:			return QString("IHM_OBJ_TxtSpare31");		
	case IHM_OBJ_TxtSpare32:			return QString("IHM_OBJ_TxtSpare32");			
	case IHM_OBJ_TxtSpare33:			return QString("IHM_OBJ_TxtSpare33");
	case IHM_OBJ_TxtSpare34:			return QString("IHM_OBJ_TxtSpare34");
	case IHM_OBJ_TxtSpare35:			return QString("IHM_OBJ_TxtSpare35");
	case IHM_OBJ_TxtSpare36:			return QString("IHM_OBJ_TxtSpare36");
	case IHM_OBJ_TxtSpare37:			return QString("IHM_OBJ_TxtSpare37");
	case IHM_OBJ_TxtSpare38:			return QString("IHM_OBJ_TxtSpare38");
	case IHM_OBJ_TxtSpare39:			return QString("IHM_OBJ_TxtSpare39");
	case IHM_OBJ_TxtSpare40:			return QString("IHM_OBJ_TxtSpare40");
	case IHM_OBJ_TxtSpare41:			return QString("IHM_OBJ_TxtSpare41");		
	case IHM_OBJ_TxtSpare42:			return QString("IHM_OBJ_TxtSpare42");			
	case IHM_OBJ_TxtSpare43:			return QString("IHM_OBJ_TxtSpare43");
	case IHM_OBJ_TxtSpare44:			return QString("IHM_OBJ_TxtSpare44");
	case IHM_OBJ_TxtSpare45:			return QString("IHM_OBJ_TxtSpare45");
	case IHM_OBJ_TxtSpare46:			return QString("IHM_OBJ_TxtSpare46");
	case IHM_OBJ_TxtSpare47:			return QString("IHM_OBJ_TxtSpare47");
	case IHM_OBJ_TxtSpare48:			return QString("IHM_OBJ_TxtSpare48");
	case IHM_OBJ_TxtSpare49:			return QString("IHM_OBJ_TxtSpare49");
	case IHM_OBJ_TxtSpare50:			return QString("IHM_OBJ_TxtSpare50");
	case IHM_OBJ_TxtSpare51:			return QString("IHM_OBJ_TxtSpare51");		
	case IHM_OBJ_TxtSpare52:			return QString("IHM_OBJ_TxtSpare52");			
	case IHM_OBJ_TxtSpare53:			return QString("IHM_OBJ_TxtSpare53");
	case IHM_OBJ_TxtSpare54:			return QString("IHM_OBJ_TxtSpare54");
	case IHM_OBJ_TxtSpare55:			return QString("IHM_OBJ_TxtSpare55");
	case IHM_OBJ_TxtSpare56:			return QString("IHM_OBJ_TxtSpare56");
	case IHM_OBJ_TxtSpare57:			return QString("IHM_OBJ_TxtSpare57");
	case IHM_OBJ_TxtSpare58:			return QString("IHM_OBJ_TxtSpare58");
	case IHM_OBJ_TxtSpare59:			return QString("IHM_OBJ_TxtSpare59");
	case IHM_OBJ_TxtSpare60:			return QString("IHM_OBJ_TxtSpare60");

	case IHM_OBJ_TransactionNbr:		return QString("IHM_OBJ_TransactionNbr");		
	case IHM_OBJ_TransactionNbrTitle:	return QString("IHM_OBJ_TransactionNbrTitle");
	case IHM_OBJ_ReceiptNbr:			return QString("IHM_OBJ_ReceiptNbr");
	case IHM_OBJ_ReceiptNbrTitle:		return QString("IHM_OBJ_ReceiptNbrTitle");
	case IHM_OBJ_Axles:					return QString("IHM_OBJ_Axles");
	case IHM_OBJ_AxlesTitle:			return QString("IHM_OBJ_AxlesTitle");
	case IHM_OBJ_Balance:				return QString("IHM_OBJ_Balance");	
	case IHM_OBJ_BalanceTitle:			return QString("IHM_OBJ_BalanceTitle");
	case IHM_OBJ_Class:					return QString("IHM_OBJ_Class");              
	case IHM_OBJ_ClassTitle:			return QString("IHM_OBJ_ClassTitle"); 
	case IHM_OBJ_AdditionalRevenue:		return QString("IHM_OBJ_AdditionalRevenue");              
	case IHM_OBJ_AdditionalRevenueTitle:return QString("IHM_OBJ_AdditionalRevenueTitle"); 

	case IHM_OBJ_NbCar:					return QString("IHM_OBJ_NbCar");				
	case IHM_OBJ_Entry:					return QString("IHM_OBJ_Entry");
	case IHM_OBJ_EntryTitle:			return QString("IHM_OBJ_EntryTitle");
	case IHM_OBJ_Currency:				return QString("IHM_OBJ_Currency");
	case IHM_OBJ_CurrencyTitle:			return QString("IHM_OBJ_CurrencyTitle");
	case IHM_OBJ_Fare:					return QString("IHM_OBJ_Fare");
	case IHM_OBJ_FareTitle:				return QString("IHM_OBJ_FareTitle");		
	case IHM_OBJ_Sale:					return QString("IHM_OBJ_Sale");
	case IHM_OBJ_SaleTitle:				return QString("IHM_OBJ_SaleTitle");
	case IHM_OBJ_SaleDue:				return QString("IHM_OBJ_SaleDue");			
	case IHM_OBJ_SaleDueTitle:			return QString("IHM_OBJ_SaleDueTitle");	
	case IHM_OBJ_SaleDuePaid:			return QString("IHM_OBJ_SaleDuePaid");			
	case IHM_OBJ_TransactionType:		return QString("IHM_OBJ_TransactionType");
	case IHM_OBJ_TransactionTypeTitle:	return QString("IHM_OBJ_TransactionTypeTitle");
	case IHM_OBJ_LaneName:				return QString("IHM_OBJ_LaneName");          
	case IHM_OBJ_LaneName1:				return QString("IHM_OBJ_LaneName1");          
	case IHM_OBJ_LaneName2:				return QString("IHM_OBJ_LaneName2");          
	case IHM_OBJ_LaneNameTitle:			return QString("IHM_OBJ_LaneNameTitle");	
	case IHM_OBJ_FareType:				return QString("IHM_OBJ_FareType");
	case IHM_OBJ_FareTypeTitle:			return QString("IHM_OBJ_FareTypeTitle");

	case IHM_OBJ_TxtModeTitle:			return QString("IHM_OBJ_TxtModeTitle");		
	case IHM_OBJ_TxtMode:				return QString("IHM_OBJ_TxtMode");		
	case IHM_OBJ_TxtCollectorTitle:		return QString("IHM_OBJ_TxtCollectorTitle");		
	case IHM_OBJ_TxtInCtrlUserTitle:	return QString("IHM_OBJ_TxtInCtrlUserTitle");		
	case IHM_OBJ_TxtInCtrlUser:			return QString("IHM_OBJ_TxtInCtrlUser");
	case IHM_OBJ_TxtInCtrlUser1:		return QString("IHM_OBJ_TxtInCtrlUser1");
	case IHM_OBJ_TxtTrsContainerTitle:	return QString("IHM_OBJ_TxtTrsContainerTitle");		
	case IHM_OBJ_TxtAlarmsTitle:		return QString("IHM_OBJ_TxtAlarmsTitle");		
	case IHM_OBJ_TxtTrsHistoryTitle:	return QString("IHM_OBJ_TxtTrsHistoryTitle");		
	case IHM_OBJ_TxtAlertesTitle:		return QString("IHM_OBJ_TxtAlertesTitle");		
	case IHM_OBJ_TxtCamNumPic:			return QString("IHM_OBJ_TxtCamNumPic");
	case IHM_OBJ_TxtScanNumPic:			return QString("IHM_OBJ_TxtScanNumPic");
	case IHM_OBJ_TxtNbCarSAS:			return QString("IHM_OBJ_TxtNbCarSAS");

// Icons
	case IHM_OBJ_IcoLaneStatus:			return QString("IHM_OBJ_IcoLaneStatus");
	case IHM_OBJ_IcoLaneMode:			return QString("IHM_OBJ_IcoLaneMode");
	case IHM_OBJ_IcoCollector:			return QString("IHM_OBJ_IcoCollector");       
	case IHM_OBJ_IcoEntryGate:			return QString("IHM_OBJ_IcoEntryGate");		
	case IHM_OBJ_IcoExitGate:			return QString("IHM_OBJ_IcoExitGate");		
	case IHM_OBJ_IcoCar:				return QString("IHM_OBJ_IcoCar");
	case IHM_OBJ_IcoTrafficLight:		return QString("IHM_OBJ_IcoTrafficLight");
	case IHM_OBJ_IcoViolation:			return QString("IHM_OBJ_IcoViolation");       
	case IHM_OBJ_IcoEntryLoop:			return QString("IHM_OBJ_IcoEntryLoop");		
	case IHM_OBJ_IcoExitLoop:			return QString("IHM_OBJ_IcoExitLoop");
	case IHM_OBJ_IcoBeacon:				return QString("IHM_OBJ_IcoBeacon");
	case IHM_OBJ_IcoEntOpticalBarrier:	return QString("IHM_OBJ_IcoEntOpticalBarrier");
	case IHM_OBJ_IcoSpare1:				return QString("IHM_OBJ_IcoSpare1");
	case IHM_OBJ_IcoSpare2:				return QString("IHM_OBJ_IcoSpare2");
	case IHM_OBJ_IcoSpare3:				return QString("IHM_OBJ_IcoSpare3");	
	case IHM_OBJ_IcoSpare4:				return QString("IHM_OBJ_IcoSpare4");			
	case IHM_OBJ_IcoSpare5:				return QString("IHM_OBJ_IcoSpare5");
	case IHM_OBJ_IcoSpare6:				return QString("IHM_OBJ_IcoSpare6");
	case IHM_OBJ_IcoSpare7:				return QString("IHM_OBJ_IcoSpare7");
	case IHM_OBJ_IcoSpare8:				return QString("IHM_OBJ_IcoSpare8");
	case IHM_OBJ_IcoSpare9:				return QString("IHM_OBJ_IcoSpare9");
	case IHM_OBJ_IcoSpare10:			return QString("IHM_OBJ_IcoSpare10");
	case IHM_OBJ_IcoSpare11:				return QString("IHM_OBJ_IcoSpare11");
	case IHM_OBJ_IcoSpare12:				return QString("IHM_OBJ_IcoSpare12");
	case IHM_OBJ_IcoSpare13:				return QString("IHM_OBJ_IcoSpare13");
	case IHM_OBJ_IcoSpare14:				return QString("IHM_OBJ_IcoSpare14");
	case IHM_OBJ_IcoSpare15:				return QString("IHM_OBJ_IcoSpare15");
	case IHM_OBJ_IcoSpare16:				return QString("IHM_OBJ_IcoSpare16");
	case IHM_OBJ_IcoSpare17:				return QString("IHM_OBJ_IcoSpare17");
	case IHM_OBJ_IcoSpare18:				return QString("IHM_OBJ_IcoSpare18");
	case IHM_OBJ_IcoSpare19:				return QString("IHM_OBJ_IcoSpare19");
	case IHM_OBJ_IcoSpare20:				return QString("IHM_OBJ_IcoSpare20");
	case IHM_OBJ_IcoSpare21:				return QString("IHM_OBJ_IcoSpare21");
	case IHM_OBJ_IcoSpare22:				return QString("IHM_OBJ_IcoSpare22");
	case IHM_OBJ_IcoSpare23:				return QString("IHM_OBJ_IcoSpare23");
	case IHM_OBJ_IcoSpare24:				return QString("IHM_OBJ_IcoSpare24");
	case IHM_OBJ_IcoSpare25:				return QString("IHM_OBJ_IcoSpare25");
	case IHM_OBJ_IcoSpare26:				return QString("IHM_OBJ_IcoSpare26");
	case IHM_OBJ_IcoSpare27:				return QString("IHM_OBJ_IcoSpare27");
	case IHM_OBJ_IcoSpare28:				return QString("IHM_OBJ_IcoSpare28");
	case IHM_OBJ_IcoSpare29:				return QString("IHM_OBJ_IcoSpare29");
	case IHM_OBJ_IcoSpare30:				return QString("IHM_OBJ_IcoSpare30");
	case IHM_OBJ_IcoSpare31:				return QString("IHM_OBJ_IcoSpare31");
	case IHM_OBJ_IcoSpare32:				return QString("IHM_OBJ_IcoSpare32");
	case IHM_OBJ_IcoSpare33:				return QString("IHM_OBJ_IcoSpare33");
	case IHM_OBJ_IcoSpare34:				return QString("IHM_OBJ_IcoSpare34");
	case IHM_OBJ_IcoSpare35:				return QString("IHM_OBJ_IcoSpare35");
	case IHM_OBJ_IcoSpare36:				return QString("IHM_OBJ_IcoSpare36");
	case IHM_OBJ_IcoSpare37:				return QString("IHM_OBJ_IcoSpare37");
	case IHM_OBJ_IcoSpare38:				return QString("IHM_OBJ_IcoSpare38");
	case IHM_OBJ_IcoSpare39:				return QString("IHM_OBJ_IcoSpare39");
	case IHM_OBJ_IcoSpare40:				return QString("IHM_OBJ_IcoSpare40");
	case IHM_OBJ_IcoSpare41:				return QString("IHM_OBJ_IcoSpare41");
	case IHM_OBJ_IcoSpare42:				return QString("IHM_OBJ_IcoSpare42");
	case IHM_OBJ_IcoSpare43:				return QString("IHM_OBJ_IcoSpare43");
	case IHM_OBJ_IcoSpare44:				return QString("IHM_OBJ_IcoSpare44");
	case IHM_OBJ_IcoSpare45:				return QString("IHM_OBJ_IcoSpare45");
	case IHM_OBJ_IcoSpare46:				return QString("IHM_OBJ_IcoSpare46");
	case IHM_OBJ_IcoSpare47:				return QString("IHM_OBJ_IcoSpare47");
	case IHM_OBJ_IcoSpare48:				return QString("IHM_OBJ_IcoSpare48");
	case IHM_OBJ_IcoSpare49:				return QString("IHM_OBJ_IcoSpare49");
	case IHM_OBJ_IcoSpare50:				return QString("IHM_OBJ_IcoSpare50");
	case IHM_OBJ_IcoCamera1:			return QString("IHM_OBJ_IcoCamera1");
	case IHM_OBJ_IcoCamera2:			return QString("IHM_OBJ_IcoCamera2");

	case IHM_OBJ_IcoBeacon2:			return QString("IHM_OBJ_IcoBeacon2");
	case IHM_OBJ_IcoSignalLight:		return QString("IHM_OBJ_IcoSignalLight");
	case IHM_OBJ_IcoExtOpticalBarrier:	return QString("IHM_OBJ_IcoExtOpticalBarrier");


// Instruction 
	case IHM_OBJ_ErrPayment:	return QString("IHM_OBJ_ErrPayment");		
	case IHM_OBJ_Payment:		return QString("IHM_OBJ_Payment");
	case IHM_OBJ_Remark:		return QString("IHM_OBJ_Remark");
	case IHM_OBJ_Instruction:	return QString("IHM_OBJ_Instruction");   
	case IHM_OBJ_Instruction2:	return QString("IHM_OBJ_Instruction2");
	case IHM_OBJ_CurrentDate:	return QString("IHM_OBJ_CurrentDate");	

//lists
	case IHM_OBJ_LstAlarms:		return QString("IHM_OBJ_LstAlarms");
	case IHM_OBJ_LstWarnings:	return QString("IHM_OBJ_LstWarnings");

//dynamic image (specific icon)
	case IHM_OBJ_Image:		return QString("IHM_OBJ_Image");	
	case IHM_OBJ_Image2:	return QString("IHM_OBJ_Image2");	

//buttons
	case IHM_OBJ_btnClass:			return QString("IHM_OBJ_btnClass");
	case IHM_OBJ_btnEntryPoint:		return QString("IHM_OBJ_btnEntryPoint");
	case IHM_OBJ_btnEntryPointNbr:	return QString("IHM_OBJ_btnEntryPointNbr");
	case IHM_OBJ_btnMode:			return QString("IHM_OBJ_btnMode");
	case IHM_OBJ_btnArrowOn:		return QString("IHM_OBJ_btnArrowOn");
	case IHM_OBJ_btnArrowOff:		return QString("IHM_OBJ_btnArrowOff");
	case IHM_OBJ_btnCrossOn:		return QString("IHM_OBJ_btnCrossOn");
	case IHM_OBJ_btnCrossOff:		return QString("IHM_OBJ_btnCrossOff");
	case IHM_OBJ_btnDsrcOn:			return QString("IHM_OBJ_btnDsrcOn");
	case IHM_OBJ_btnDsrcOff:		return QString("IHM_OBJ_btnDsrcOff");
	case IHM_OBJ_btnMagneticOn:		return QString("IHM_OBJ_btnMagneticOn");
	case IHM_OBJ_btnMagneticOff:	return QString("IHM_OBJ_btnMagneticOff");
	case IHM_OBJ_btnCollectorOn:	return QString("IHM_OBJ_btnCollectorOn");
	case IHM_OBJ_btnCollectorOff:	return QString("IHM_OBJ_btnCollectorOff");

	case IHM_OBJ_btnCamera:return QString("IHM_OBJ_btnCamera");
	case IHM_OBJ_btnScanner:return QString("IHM_OBJ_btnScanner");
	case IHM_OBJ_btnComment:return QString("IHM_OBJ_btnComment");

	case IHM_OBJ_btnGabarite:return QString("IHM_OBJ_btnGabarite");
	case IHM_OBJ_btnEntryGate:return QString("IHM_OBJ_btnEntryGate");
	case IHM_OBJ_btnExitGate:return QString("IHM_OBJ_btnExitGate");
	case IHM_OBJ_btnCar3:return QString("IHM_OBJ_btnCar3");
	case IHM_OBJ_btnCar2:return QString("IHM_OBJ_btnCar2");
	case IHM_OBJ_btnCar:return QString("IHM_OBJ_btnCar");
	case IHM_OBJ_btnEntryLoop:return QString("IHM_OBJ_btnEntryLoop");
	case IHM_OBJ_btnExitLoop:return QString("IHM_OBJ_btnExitLoop");
	case IHM_OBJ_btnTblBeacon:return QString("IHM_OBJ_btnTblBeacon");

	case IHM_OBJ_btnCamGrab:return QString("IHM_OBJ_btnCamGrab");
	case IHM_OBJ_btnCamHide:return QString("IHM_OBJ_btnCamHide");
	case IHM_OBJ_btnScanGrab:return QString("IHM_OBJ_btnScanGrab");
	case IHM_OBJ_btnScanHide:return QString("IHM_OBJ_btnScanHide");	
	case IHM_OBJ_btnScanSend:return QString("IHM_OBJ_btnScanSend");
	case IHM_OBJ_btnScanSave:return QString("IHM_OBJ_btnScanSave");
	case IHM_OBJ_btnCommHide:return QString("IHM_OBJ_btnCommHide");
	case IHM_OBJ_btnExternalDisplay:return QString("IHM_OBJ_btnExternalDisplay");
	case IHM_OBJ_btnExternalDisplay2:return QString("IHM_OBJ_btnExternalDisplay2");
	case IHM_OBJ_btnTakeControl:return QString("IHM_OBJ_btnTakeControl");
	case IHM_OBJ_btnReturnControl:return QString("IHM_OBJ_btnReturnControl");

//spare buttons
	case IHM_OBJ_Product1 :return QString("IHM_OBJ_Product1");
	case IHM_OBJ_Product2:return QString("IHM_OBJ_Product2");
	case IHM_OBJ_Product3:return QString("IHM_OBJ_Product3");
	case IHM_OBJ_Product4:return QString("IHM_OBJ_Product4");
	case IHM_OBJ_Product5:return QString("IHM_OBJ_Product5");	

	case IHM_OBJ_btnSpeedLimitOn:return QString("IHM_OBJ_btnSpeedLimitOn");	
	case IHM_OBJ_btnSpeedLimitOff:return QString("IHM_OBJ_btnSpeedLimitOff");	
	case IHM_OBJ_btnWarningOn:return QString("IHM_OBJ_btnWarningOn");	
	case IHM_OBJ_btnWarningOff:return QString("IHM_OBJ_btnWarningOff");	
	case IHM_OBJ_btnDsrcTSAOn:return QString("IHM_OBJ_btnDsrcTSAOn");
	case IHM_OBJ_btnDsrcTSAOff:return QString("IHM_OBJ_btnDsrcTSAOff");

	case IHM_OBJ_btnSpare1:return QString("IHM_OBJ_btnSpare1");
	case IHM_OBJ_btnSpare2:return QString("IHM_OBJ_btnSpare2");
	case IHM_OBJ_btnSpare3:return QString("IHM_OBJ_btnSpare3");
	case IHM_OBJ_btnSpare4:return QString("IHM_OBJ_btnSpare4");
	case IHM_OBJ_btnSpare5:return QString("IHM_OBJ_btnSpare5");
	case IHM_OBJ_btnSpare6:return QString("IHM_OBJ_btnSpare6");
	case IHM_OBJ_btnSpare7:return QString("IHM_OBJ_btnSpare7");
	case IHM_OBJ_btnSpare8:return QString("IHM_OBJ_btnSpare8");
	case IHM_OBJ_btnSpare9:return QString("IHM_OBJ_btnSpare9");
	case IHM_OBJ_btnSpare10:return QString("IHM_OBJ_btnSpare10");

	case IHM_OBJ_btnSpare11:return QString("IHM_OBJ_btnSpare11");
	case IHM_OBJ_btnSpare12:return QString("IHM_OBJ_btnSpare12");
	case IHM_OBJ_btnSpare13:return QString("IHM_OBJ_btnSpare13");
	case IHM_OBJ_btnSpare14:return QString("IHM_OBJ_btnSpare14");
	case IHM_OBJ_btnSpare15:return QString("IHM_OBJ_btnSpare15");
	case IHM_OBJ_btnSpare16:return QString("IHM_OBJ_btnSpare16");
	case IHM_OBJ_btnSpare17:return QString("IHM_OBJ_btnSpare17");
	case IHM_OBJ_btnSpare18:return QString("IHM_OBJ_btnSpare18");
	case IHM_OBJ_btnSpare19:return QString("IHM_OBJ_btnSpare19");
	case IHM_OBJ_btnSpare20:return QString("IHM_OBJ_btnSpare20");

	case IHM_OBJ_btnSpare21:return QString("IHM_OBJ_btnSpare21");
	case IHM_OBJ_btnSpare22:return QString("IHM_OBJ_btnSpare22");
	case IHM_OBJ_btnSpare23:return QString("IHM_OBJ_btnSpare23");
	case IHM_OBJ_btnSpare24:return QString("IHM_OBJ_btnSpare24");
	case IHM_OBJ_btnSpare25:return QString("IHM_OBJ_btnSpare25");
	case IHM_OBJ_btnSpare26:return QString("IHM_OBJ_btnSpare26");
	case IHM_OBJ_btnSpare27:return QString("IHM_OBJ_btnSpare27");
	case IHM_OBJ_btnSpare28:return QString("IHM_OBJ_btnSpare28");
	case IHM_OBJ_btnSpare29:return QString("IHM_OBJ_btnSpare29");
	case IHM_OBJ_btnSpare30:return QString("IHM_OBJ_btnSpare30");



	case IHM_OBJ_TableView1:return QString("IHM_OBJ_TableView1");
	case IHM_OBJ_TableView2:return QString("IHM_OBJ_TableView2");
	case IHM_OBJ_TableView3:return QString("IHM_OBJ_TableView3");

//horizontal menu
	case IHM_OBJ_HMenuView1:return QString("IHM_OBJ_HMenuView1");
	case IHM_OBJ_HMenuView2:return QString("IHM_OBJ_HMenuView2");

	case IHM_OBJ_RVideoView1:return QString("IHM_OBJ_RVideoView1");
	case IHM_OBJ_RVideoView2:return QString("IHM_OBJ_RVideoView2");

//to be able to dynamicaly show hide containers
	case IHM_OBJ_CNT_HeaderGroup:		return QString("IHM_OBJ_CNT_HeaderGroup");
	case IHM_OBJ_CNT_InstructionGroup:	return QString("IHM_OBJ_CNT_InstructionGroup");
	case IHM_OBJ_CNT_ModeGroup:			return QString("IHM_OBJ_CNT_ModeGroup");	
	case IHM_OBJ_CNT_PaymentGroup:		return QString("IHM_OBJ_CNT_PaymentGroup");	
	case IHM_OBJ_CNT_TrsGroup:			return QString("IHM_OBJ_CNT_TrsGroup");	

//We cannot use spare containers since these are 
// not visible when not in control
	case IHM_OBJ_CNT_Comments:return QString("IHM_OBJ_CNT_Comments");
	case IHM_OBJ_CNT_Camera:return QString("IHM_OBJ_CNT_Camera");
	case IHM_OBJ_CNT_Scanner:return QString("IHM_OBJ_CNT_Scanner");

	case IHM_OBJ_CNT_Spare1:return QString("IHM_OBJ_CNT_Spare1");
	case IHM_OBJ_CNT_Spare2:return QString("IHM_OBJ_CNT_Spare2");
	case IHM_OBJ_CNT_Spare3:return QString("IHM_OBJ_CNT_Spare3");
	case IHM_OBJ_CNT_Spare4:return QString("IHM_OBJ_CNT_Spare4");
	case IHM_OBJ_CNT_Spare5:return QString("IHM_OBJ_CNT_Spare5");
	case IHM_OBJ_CNT_Spare6:return QString("IHM_OBJ_CNT_Spare6");
	case IHM_OBJ_CNT_Spare7:return QString("IHM_OBJ_CNT_Spare7");
	case IHM_OBJ_CNT_Spare8:return QString("IHM_OBJ_CNT_Spare8");
	case IHM_OBJ_CNT_Spare9:return QString("IHM_OBJ_CNT_Spare9");
	case IHM_OBJ_CNT_Spare10:return QString("IHM_OBJ_CNT_Spare10");
	case IHM_OBJ_CNT_Spare11:return QString("IHM_OBJ_CNT_Spare11");
	case IHM_OBJ_CNT_Spare12:return QString("IHM_OBJ_CNT_Spare12");
	case IHM_OBJ_CNT_Spare13:return QString("IHM_OBJ_CNT_Spare13");
	case IHM_OBJ_CNT_Spare14:return QString("IHM_OBJ_CNT_Spare14");
	case IHM_OBJ_CNT_Spare15:return QString("IHM_OBJ_CNT_Spare15");
	case IHM_OBJ_CNT_Spare16:return QString("IHM_OBJ_CNT_Spare16");
	case IHM_OBJ_CNT_Spare17:return QString("IHM_OBJ_CNT_Spare17");
	case IHM_OBJ_CNT_Spare18:return QString("IHM_OBJ_CNT_Spare18");
	case IHM_OBJ_CNT_Spare19:return QString("IHM_OBJ_CNT_Spare19");
	case IHM_OBJ_CNT_Spare20:return QString("IHM_OBJ_CNT_Spare20");

	case IHM_OBJ_CNT_MiniWeb:return QString("IHM_OBJ_CNT_MiniWeb");
	case IHM_OBJ_CNT_AlarmsGroup:return QString("IHM_OBJ_CNT_AlarmsGroup");

	case IHM_OBJ_TabControl1:return QString("IHM_OBJ_TabControl1");
	case IHM_OBJ_TabControl2:return QString("IHM_OBJ_TabControl2");
	case IHM_OBJ_TabControl3:return QString("IHM_OBJ_TabControl3");
	case IHM_OBJ_TabControl4:return QString("IHM_OBJ_TabControl4");

	case IHM_OBJ_OpenLink:return QString("IHM_OBJ_OpenLink");
	case IHM_OBJ_OpenLoginDlg:return QString("IHM_OBJ_OpenLoginDlg");
	case IHM_OBJ_AboutDlg:return QString("IHM_OBJ_AboutDlg");		
	
	
	default:
		return "IHM_OBJ_Unknown";
	    break;
	}
}


int MHelpFuncs::getObjectIdFromString(QString sObjectName)
{

	for (int i = IHM_OBJ_All_Object; i<=IHM_OBJ_AboutDlg; i++)
	{
		if(QString::compare(sObjectName, getStringFromObjectId(i), Qt::CaseInsensitive)==0)	
		{
			return i;
		}	
	}

	return IHM_OBJ;
}

int MHelpFuncs::convertAlignment(QString sAlign)
{
	int iAlignment; 

	if(sAlign.compare("center", Qt::CaseInsensitive) ==0)
		iAlignment = Qt::AlignHCenter | Qt::AlignVCenter;
	else if(sAlign.compare("left", Qt::CaseInsensitive) ==0)
		iAlignment = Qt::AlignLeft| Qt::AlignVCenter;
	else if(sAlign.compare("right", Qt::CaseInsensitive) ==0)
		iAlignment = Qt::AlignRight| Qt::AlignVCenter;
	else
		iAlignment = Qt::AlignHCenter| Qt::AlignVCenter;

	return iAlignment;
}

QString MHelpFuncs::deepCopy(const QString &right)
{ 
	return QString::fromUtf16(right.utf16());
}

QRect MHelpFuncs::convertStrToRect(const QString& sGeometry)
{
	QStringList sLst = sGeometry.split(',');
	QRect retValue;
	QString sCurrent;
	bool bOK;

	for(int i=0;i<sLst.size();i++)
	{
		sCurrent = sLst.at(i);
		
		if(sCurrent.isEmpty())
			continue;

		switch(i)
		{
			case 0:
			{
				int x = sCurrent.toInt(&bOK);

				if(bOK)
					retValue.setX(x);
			} 
			break;
			case 1:
			{
				int y = sCurrent.toInt(&bOK);

				if(bOK)
					retValue.setY(y);
			}
			break;
			case 2:
			{
				int width = sCurrent.toInt(&bOK);

				if(bOK)
					retValue.setWidth(width);
			}
			break;
			case 3:
			{
				int height = sCurrent.toInt(&bOK);

				if(bOK)
					retValue.setHeight(height);
			}
			default:
				break;
		}

	}



	return retValue;
}

QString MHelpFuncs::cleanAbsolutePath(QString sPath)
{
	QString sRet;

	sPath = sPath.trimmed();

	if (sPath.size() > 0)
		if (sPath.right(1) == QString("/") || sPath.right(1) == QString("\\"))
			sPath = sPath.left(sPath.size() - 1);
	
	
	if (sPath.at(0) == QChar('.') && sPath.at(1) != QChar('.'))
		sRet = QDir::cleanPath(QDir::currentPath() + QDir::separator() + QDir::cleanPath(sPath));
	else
		sRet = QDir::cleanPath(sPath);


	return sRet;
}
