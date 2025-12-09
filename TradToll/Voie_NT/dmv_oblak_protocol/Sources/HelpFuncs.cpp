#include <QFile>
#include <QTextStream>
#include <QStringList>

#include "HelpFuncs.h"

QString HelpFuncs::loadFileContent(QString sFilePath)
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

QByteArray HelpFuncs::StrToByteArray(const QString & sData)
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

QString HelpFuncs::ByteArrayToAnsi(const QByteArray & baData)
{
	QString sRetVal;
	int iLen,i;
	char cByteVal;
	QString sTemp;
	
	iLen = baData.size();

	for(i=0; i < iLen; i++)
	{
		cByteVal = baData[i];
		
		if(cByteVal>=32 || cByteVal==0x0D || cByteVal == 0x0A || cByteVal == 0x09)
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

QString HelpFuncs::ByteArrayToVisibleAnsi(const QByteArray & baData)
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

QString HelpFuncs::HexStrToBinStr(const QString & sData)
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

QString HelpFuncs::ByteArrayToHexStr(const QByteArray & baData)
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

QByteArray HelpFuncs::HexStrToByteArray(const QString & sData)
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

QString HelpFuncs::RemoveSpacesFromHexStr(const QString & sData)
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

QString HelpFuncs::AddSpacesToHexStr(const QString & sData)
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

QByteArray HelpFuncs::ByteArrayToHexArray(const QByteArray & baData)
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

QByteArray HelpFuncs::HexArrayToByteArray(const QByteArray & baData)
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

QString HelpFuncs::escapeStringForSprintf(const QString & sData)
{
	QString sResult = sData;

	sResult = sResult.replace('%', "%%");

	return sResult;
}