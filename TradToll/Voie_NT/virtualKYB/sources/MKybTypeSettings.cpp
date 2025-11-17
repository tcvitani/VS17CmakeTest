

#include "MKybTypeSettings.h"
#include "MTracer.h"
#include "MTracer.h"

#include <QFile>
#include <QDir>


#define MKYB_CFG_PREFIX_GUI "GUI" 
#define MKYB_CFG_PREFIX_KEY "KEY" 
#define MKYB_CFG_PREFIX_POS "POS"
#define MKYB_CFG_PREFIX_TITLE_BAR "TITLE_BAR"
#define MKYB_CFG_PREFIX_COMMENT '#' 
#define MKYB_CFG_KEY_EVENT_SEPARATOR '|'
#define MKYB_CFG_KEY_EVENT_CODE_SEPARATOR ','
#define MKYB_CFG_KEY_OBJName_SEPARATOR ':'


//----------------------------------------------------------------------------

MKybKeyCfg::~MKybKeyCfg()
{
	while (!m_lstKybEvents.isEmpty())
		 delete m_lstKybEvents.takeFirst();

}

MKybKeyCfg& MKybKeyCfg::operator=(const MKybKeyCfg& right)
{
	const MKybKeyCfg *pD = &right;

	Q_ASSERT(pD!=NULL);

	m_sWidgetName = pD->m_sWidgetName;

	while (!m_lstKybEvents.isEmpty())
		 delete m_lstKybEvents.takeFirst();

	//make a commands list copy
	KybEvent * pNew;
	KybEvent * pCurrent;

	for(int i=0; i<pD->m_lstKybEvents.count(); i++)
	{
		pCurrent = pD->m_lstKybEvents.at(i);
		pNew = new KybEvent();
		*pNew = *pCurrent;
		m_lstKybEvents.append(pNew);
	}

	return *this;
}

	
bool MKybKeyCfg::appendKeyEventsString(QByteArray baCfgLine)
{
	bool bRetVal = false;

	QByteArray baCfgLineTrimmed = baCfgLine.trimmed();

	QList<QByteArray> sLst = baCfgLineTrimmed.split(MKYB_CFG_KEY_EVENT_SEPARATOR);

	
	for(int i=0;i<sLst.size();i++)
	{
		QByteArray baCurrent = sLst.at(i).trimmed();

		if(baCurrent.size()>0)
		{
			if((baCurrent.indexOf('{') == 0) && (baCurrent.indexOf('}')==(baCurrent.size()-1)))
			{
				baCurrent = baCurrent.mid(1, baCurrent.size()-2);
			}

			QByteArray baParam1,baParam2;
			
			//0x20, 0x0002

			int iPos = baCurrent.indexOf(MKYB_CFG_KEY_EVENT_CODE_SEPARATOR);
			if(iPos>=0)
			{
				baParam1 = baCurrent.left(iPos);
				baParam2 = baCurrent.mid(iPos+1);
			}
			else
			{
				baParam1 = baCurrent;
				baParam2 = "0";
			}

			bool bOK;
			uint uiVK = baParam1.toUInt(&bOK,0);

			if(bOK)
			{
				uint uiFlags = baParam2.toUInt(&bOK,0);
				KybEvent *p = new KybEvent();
				p->bVk = (quint8)uiVK%256;
				p->dwFlags = (quint32) uiFlags;
				m_lstKybEvents.append(p);
				
				bRetVal = true;
			}
			else
			{
				TRACE_W(QString( "MKybTypeSettings::appendKeyEventsString: Error parsing key events configuration [%1]!")
					.arg(baCfgLine.data()));
				return false;
			}
				
		}
	}

	return bRetVal;

}

//----------------------------------------------------------------------------





MKybTypeSettings::MKybTypeSettings()
{
	m_bShowTitleBar = true;
}


MKybTypeSettings::~MKybTypeSettings()
{
	 clearData();
}

void MKybTypeSettings::clearData()
{
	 while (!m_lstKybKeys.isEmpty())
		 delete m_lstKybKeys.takeFirst();
}

bool MKybTypeSettings::loadKeybSettings(QString sCfgID, QString sCfgFilePath)
{
	bool bRetValue = true; 
	clearData();
	m_sCfgID = sCfgID;	
	m_sCfgFile = sCfgFilePath;

	QFileInfo fi(m_sCfgFile);
	QByteArray baBuffer;

	if(fi.exists())
	{
		QFile file(m_sCfgFile);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
		
			while (!file.atEnd()) {
				QByteArray line = file.readLine();
				bRetValue = bRetValue && processCfgLine(line);

				if(!bRetValue)
				{
					TRACE_W(QString( "MKybTypeSettings::loadKeybSettings: Error parsing line: [%1] in file[%2]!")
						.arg(line.data()).arg(sCfgFilePath));
					break;
				}

			}

			file.close();
		}
		else
			bRetValue = false;

	}
	else
	{
		TRACE_W(QString( "MKybTypeSettings::loadKeybSettings: File not found: [%1]!")
			.arg(m_sCfgFile));
		bRetValue = false;
	}
	
	return bRetValue;
}


bool MKybTypeSettings::processCfgLine(QByteArray baLine)
{
	bool bRetVal = false;

	QByteArray baLineTrimmed = baLine; 
	int iPos = baLineTrimmed.indexOf(MKYB_CFG_PREFIX_COMMENT);

	if(iPos>=0)
	{
		baLineTrimmed = baLineTrimmed.left(iPos);
	}
	
	baLineTrimmed = baLineTrimmed.trimmed();
	
	if(baLineTrimmed.size()==0)
		bRetVal = true; //empty line or comment
	else if(baLineTrimmed.left(3).toUpper() == MKYB_CFG_PREFIX_GUI)
	{
		baLineTrimmed = baLineTrimmed.mid(4).trimmed();
		m_sUIFileNamePath = baLineTrimmed;
		bRetVal = true;
	}
	else if(baLineTrimmed.left(3).toUpper() == MKYB_CFG_PREFIX_KEY)
	{
		baLineTrimmed = baLineTrimmed.mid(4).trimmed();
				
		int iPos = baLineTrimmed.indexOf(MKYB_CFG_KEY_OBJName_SEPARATOR);
		if(iPos >=0)
		{
			QString sWidgetName = baLineTrimmed.left(iPos).trimmed(); //extract object name
			baLineTrimmed = baLineTrimmed.mid(iPos+1);

			MKybKeyCfg * pKey = new MKybKeyCfg();
			pKey->setWidgetName(sWidgetName);
			bRetVal = pKey->appendKeyEventsString(baLineTrimmed);
			m_lstKybKeys.append(pKey);
		}
		else
			bRetVal = false;

	} 
	else if(baLineTrimmed.left(3).toUpper() == 	MKYB_CFG_PREFIX_POS) 
	{
		baLineTrimmed = baLineTrimmed.mid(4).trimmed();
		
		int iPos = baLineTrimmed.indexOf(MKYB_CFG_KEY_EVENT_CODE_SEPARATOR);
		if(iPos >=0)
		{
			QString sPosX = baLineTrimmed.left(iPos).trimmed(); //extract object name
			QString sPosY = baLineTrimmed.mid(iPos+1).trimmed();

			bool bOK, bOK2;
			m_ptInitialPos.setX(sPosX.toInt(&bOK,0));
			m_ptInitialPos.setY(sPosY.toInt(&bOK2,0));

			bRetVal = bOK && bOK2;//if both OK
		}
		else
			bRetVal = false;
	}
	else if(baLineTrimmed.left(9).toUpper() == 	MKYB_CFG_PREFIX_TITLE_BAR) 
	{
		baLineTrimmed = baLineTrimmed.mid(10).trimmed();
		
		if(baLineTrimmed!="0")
			m_bShowTitleBar = true;
		else
			m_bShowTitleBar = false;

		bRetVal = true;
	}	
	else
		bRetVal = false;

	return bRetVal;
}