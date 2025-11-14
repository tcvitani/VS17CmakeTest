

#include <QStringList>
#include <QDomDocument>
#include <QFile>

#include "MTracer.h"
#include "MHtmlCodec.h"
#include "MHelpFuncs.h"


#define XML_ELEMENT_ENC_MAP_ROOT "HTML_ENC_MAP_ROOT"
#define XML_ELEMENT_CHAR_ITEM "CHAR_ITEM"
#define XML_ATTRIBUTE_VALUE "VALUE"
#define XML_ATTRIBUTE_HTML "HTML"
#define XML_ATTRIBUTE_ALTER_HTML "ALTER_HTML"


MHtmlCodec::MHtmlCodec()
{
	m_bInitialized = false;
}

MHtmlCodec::~MHtmlCodec()
{
	while(!m_lstHtmlEncMap.isEmpty())
		delete m_lstHtmlEncMap.takeLast();

}

void MHtmlCodec::init(QString sMapFile)
{
	TRACE_WEB_D( "MHtmlCodec::init..." );

	if(sMapFile!="")
	{

		bool bRet = false;
		QString errorStr;
		int errorLine;
		int errorColumn;
		QDomDocument xmlData;

		QFile file(sMapFile);
		
		if(file.open(QFile::ReadOnly))
		{

			bRet = xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn);

			if(bRet)
			{
				QDomElement root = xmlData.documentElement();
				if(root.tagName() != XML_ELEMENT_ENC_MAP_ROOT)
				{
					TRACE_WEB_W( "MHtmlCodec::init: Invalid root element!");
					bRet = false;
				}
				else
				{
					QDomNodeList lstCharItems = root.elementsByTagName(XML_ELEMENT_CHAR_ITEM);
					QDomNode currChar;
					
					for(int i=0;i<lstCharItems.count();i++)
					{
						currChar = lstCharItems.at(i); 
						
						QString sVal = MHelpFuncs::getAttributeText(&currChar, XML_ATTRIBUTE_VALUE);
						QString sHtml = MHelpFuncs::getAttributeText(&currChar, XML_ATTRIBUTE_HTML);
						QString sAlterHtml = MHelpFuncs::getAttributeText(&currChar, XML_ATTRIBUTE_ALTER_HTML);
						
						if(sVal!="")
						{
							HtmlChar * pNew = new HtmlChar();
							pNew->chValue = sVal.at(0);
							pNew->sHtml = sHtml;
							pNew->sAlterHtml = sAlterHtml;

							m_lstHtmlEncMap.append(pNew);
							m_bInitialized = true;
						}
					}
				}
			}
			else
			{
				QString sMsg = QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
				TRACE_WEB_D( QString("MHtmlCodec::init %1").arg(sMsg));
			}
		}
		else
		{
			TRACE_WEB_D( QString("MHtmlCodec::init: Unable to open file %1").arg(sMapFile) );
		}

			
	}

}

QString MHtmlCodec::htmlEncodeStr(QString str)
{
	QString sRetVal = str;

	if(sRetVal=="")
		return sRetVal;

	if(m_bInitialized)
	{
		HtmlChar * pCurrCh;
		
		for(int j=0; j<m_lstHtmlEncMap.size();j++)
		{
			pCurrCh = m_lstHtmlEncMap.at(j);
			sRetVal = sRetVal.replace(pCurrCh->chValue, pCurrCh->sHtml);
		}

		return sRetVal;
	}
	else
		return str;

}

