

#include <QFile>
#include <QPixmap>
#include <QDomDocument>

#include "MHelpFuncs.h"
#include "MIhmConfigCursors.h"
#include "MIhmConfigImages.h"
#include "MTracer.h"


const QString MIhmConfigCursors::cCURSORS_ROOT_ELEMENT = "IHM_CONFIG_CURSORS"; 
const QString MIhmConfigCursors::cCURSOR_ELEMENT = "CURSOR"; 
const QString MIhmConfigCursors::cATTRIBUTE_ID = "ID"; // = "ID"
const QString MIhmConfigCursors::cATTRIBUTE_IMAGE = "IMAGE"; // = "IMAGE"
const QString MIhmConfigCursors::cATTRIBUTE_QT_CURSOR = "QT_CURSOR"; // = "QT_CURSOR"

MIhmConfigCursors::MIhmConfigCursors()
{
	
}


MIhmConfigCursors::~MIhmConfigCursors()
{
	while(!m_lstCursors.isEmpty())
		delete m_lstCursors.takeLast();

}



MCursor * MIhmConfigCursors::findCursor(QString sCursorID)
{
	for (int i=0; i<m_lstCursors.size();i++)
	{
		if(m_lstCursors.at(i)->m_sId == sCursorID)
			return m_lstCursors.at(i);
	}

	return NULL; //NULL
}


QCursor MIhmConfigCursors::getCursor(MIhmConfigImages *pImages, QString sCursorID)
{
	QCursor cur(Qt::ArrowCursor);

	MCursor *pMCur = findCursor(sCursorID);

	if(pMCur!=NULL)
	{
		bool bLoaded = false;

		if(pMCur->m_sImage!="")
		{
			QString sCurImgPath = pImages->getImageFullPath(pMCur->m_sImage);

			QPixmap px;
			if(px.load(sCurImgPath))
			{
				QCursor oNewPixmapCursor(px,1,1);

				bLoaded = true;

				return oNewPixmapCursor;
			}
			else
			{
				TRACE_W(QString("MIhmConfigCursors::getCursor: Unable to load cursor image %1").arg(sCurImgPath));
			}
		}
		
		if(!bLoaded && pMCur->m_iQTCursorShape<=Qt::BusyCursor)
		{
			cur.setShape((Qt::CursorShape)pMCur->m_iQTCursorShape);
		}
	}

	return cur;
}


bool MIhmConfigCursors::loadConfigFromFile(QString sCursorsFilePath)
{
	bool bRet = false;
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument xmlData;

	QFile file(sCursorsFilePath);
	
	if(file.open(QFile::ReadOnly))
	{

		bRet = xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn);

		if(bRet)
		{
			QDomElement root = xmlData.documentElement();
			if(root.tagName() != cCURSORS_ROOT_ELEMENT)
			{
				TRACE_W("MIhmConfigCursors::OpenXML: Invalid root element!");
				bRet = false;
			}
			else
			{
				QDomNodeList lstItems = root.childNodes();
				QDomNode currItem;
						
				for(int i=0;i<lstItems.count();i++)
				{
					currItem = lstItems.at(i); 
					if(currItem.nodeName()==cCURSOR_ELEMENT)
					{
						QString sCursorID = MHelpFuncs::getAttributeText(&currItem, cATTRIBUTE_ID);
						QString sCursorImage = MHelpFuncs::getAttributeText(&currItem, cATTRIBUTE_IMAGE);
						QString sQTCursorID = MHelpFuncs::getAttributeText(&currItem, cATTRIBUTE_QT_CURSOR);
						
						int iQTCUrsorShape = sQTCursorID.toInt();

						if(sQTCursorID=="")
							iQTCUrsorShape = -1;
						
						MCursor *pCur = new MCursor();
						pCur->m_sId = sCursorID;
						pCur->m_sImage = sCursorImage;
						pCur->m_iQTCursorShape = iQTCUrsorShape;

						m_lstCursors.append(pCur);

						bRet = true; //at least one should be added;
					}
				}

			}
		}
		else
		{
			QString sMsg = QString("MIhmConfigCursors::OpenXML:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
			TRACE_W(sMsg);
		}
	}
	else
	{
		TRACE_W(QString( "MIhmConfigCursors::OpenXML: Unable to open file %1").arg(sCursorsFilePath) );
	}

	return bRet;
}


