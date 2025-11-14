

#include <QColor>
#include <QFile>
#include <QDomDocument>


#include "MIhmConfigColor.h"
#include "MHelpFuncs.h"
#include "MTracer.h"

const QString MIhmConfigColor::cCOLORS_ROOT_ELEMENT = "COLORS"; 
const QString MIhmConfigColor::cCOLOR_ELEMENT = "COLOR"; 
const QString MIhmConfigColor::cATTRIBUTE_NAME = "NAME"; 
const QString MIhmConfigColor::cATTRIBUTE_VALUE = "VALUE"; 


MIhmConfigColor::MIhmConfigColor()
{
	
}


MIhmConfigColor::~MIhmConfigColor()
{

}

bool MIhmConfigColor::loadConfigFromFile(QString sFilePath)
{
	bool bRet = false;
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument xmlData;

	QFile file(sFilePath);
	
	if(file.open(QFile::ReadOnly))
	{

		bRet = xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn);

		if(bRet)
		{
			QDomElement root = xmlData.documentElement();
			if(root.tagName() != cCOLORS_ROOT_ELEMENT)
			{
				TRACE_W("MIhmConfigColor::OpenXML: Invalid root element!");
				bRet = false;
			}
			else
			{
				QDomNodeList lstItems = root.childNodes();
				QDomNode currItem;
						
				for(int i=0;i<lstItems.count();i++)
				{
					currItem = lstItems.at(i); 
					if(currItem.nodeName()==cCOLOR_ELEMENT)
					{
						QString sShiftType = MHelpFuncs::getAttributeText(&currItem, cATTRIBUTE_NAME);
						QString sShiftColour = MHelpFuncs::getAttributeText(&currItem, cATTRIBUTE_VALUE);
						
						m_mapShiftColours.insert(sShiftType, sShiftColour);

						bRet = true; //at least one should be added;
					}
				}

			}
		}
		else
		{
			QString sMsg = QString("MIhmConfigColor::OpenXML:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
			TRACE_W(sMsg);
		}
	}
	else
	{
		TRACE_W(QString("MIhmConfigColor::OpenXML: Unable to open file %1").arg(sFilePath));
	}

	return bRet;
}


bool MIhmConfigColor::getColourForShift(QString sShiftType, QString& sShiftColour)
{
	sShiftColour = m_mapShiftColours.value(sShiftType);

	if(sShiftColour=="")
		return false;
	else
		return true;
}


QString MIhmConfigColor::makeHexColorString(int iColor)
{
	int r = iColor & 0xFF;
	int g = (iColor & 0xFF00)/0x100;
	int b = (iColor & 0xFF0000)/0x10000; 

	return QString("%1%2%3")
			.arg(b,2,16,QChar('0'))
			.arg(g,2,16,QChar('0'))
			.arg(r,2,16,QChar('0'));
}

QColor MIhmConfigColor::makeColorFromHexString(QString sHexColor)
{
	QColor col;
	bool bTemp;
	bool bOK = (sHexColor.length()==6);
	
	int b = sHexColor.left(2).toInt(&bTemp,16);
	bOK = bOK && bTemp;
	int g = sHexColor.mid(2,2).toInt(&bTemp,16);
	bOK = bOK && bTemp;
	int r = sHexColor.right(2).toInt(&bTemp,16);
	bOK = bOK && bTemp;
	
	if(bOK)
		col = QColor(r,g,b); 
	else
		col = QColor(0,0,0); //set black if something is wrong

	return col;
}

QString MIhmConfigColor::createColorStyle(QString sHexColour)
{
	QString sStyle;
	
	if(sHexColour!="")
		sStyle = QString("color: #%1;").arg(sHexColour);

	return sStyle;
}


QString MIhmConfigColor::createBkgColorStyle(QString sHexColour)
{
	QString sStyle;

	if(sHexColour!="")
		sStyle = QString("background-color: #%1;").arg(sHexColour);
	
	return sStyle;
}