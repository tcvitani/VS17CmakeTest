#include <QFile>
#include <QTextStream>

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
	#include "ihm_loc.h"
}

#include "MHMenuFileLoader.h"
#include "MHelpFuncs.h"


#include "MTracer.h"



MHMenuFileLoader::MHMenuFileLoader()
{
}

MHMenuFileLoader::~MHMenuFileLoader()
{
	clearData();
}

void MHMenuFileLoader::clearData()
{
	while(!m_lstMenuItems.isEmpty())
		delete m_lstMenuItems.takeFirst();
}

bool MHMenuFileLoader::loadDataFromBuffer(QString sData, bool& bReload)
{
	if(OpenXMLBuffer(sData))
	{
		return safeReadModelFromXML(bReload);
	}
	else
	{
		TRACE_W(QString( "MHMenuFileLoader::loadDataFromFile: Error parsing data:\n%1").arg(sData));
	}

	return false;
}

bool MHMenuFileLoader::loadDataFromFile(QString sDataFilePath, bool& bReload)
{
	if(OpenXMLFile(sDataFilePath))
	{
		return safeReadModelFromXML(bReload);
	}
	else
	{
		TRACE_W(QString( "MHMenuFileLoader::loadDataFromFile: Error parsing file:%1!").arg(sDataFilePath));
	}

	return false;
}

//if the update fails do the complete menu reload
bool MHMenuFileLoader::safeReadModelFromXML(bool& bReload)
{

	if(bReload)
	{
		clearData();
		return readModelFromXML(true);
	}
	else if(!readModelFromXML(bReload))
	{
		clearData();
		bReload = true; //to give information to the QMenuBar that is using the model that complete reload is necessary
		return readModelFromXML(true);
	}
	else
		return true;

}

bool MHMenuFileLoader::readModelFromXML(bool bReload)
{
	QDomElement root = m_xmlData.documentElement();
	QDomNodeList lstItems = root.childNodes();
	QDomNode currItem;
			
	for(int i=0;i<lstItems.count();i++)
	{
		currItem = lstItems.at(i); 
		
		if(currItem.nodeName()==IHM_CFG_VAL_MENU_FILE_ITEM_ELEMENT)
		{	
			if(!bReload)
			{
				if(m_lstMenuItems.size()==lstItems.count() && m_lstMenuItems.size()>0)
				{
					MMenuModelItem * pCurrentSubItem = m_lstMenuItems.at(i);
					
					if(pCurrentSubItem==NULL) 
									return false;
					
					if(!updateMenuItem(currItem, pCurrentSubItem)) 
									return false;
				}
				else
					return false;
			}
			else
			{
				MMenuModelItem * pNewSubItem = recreateMenuItem(currItem, NULL);
				m_lstMenuItems.append(pNewSubItem);				
			}
		}
	}

	return true;
}


MMenuModelItem * MHMenuFileLoader::recreateMenuItem(QDomNode &currItemNode, MMenuModelItem * parent)
{
	MMenuModelItem * pNewItem = new MMenuModelItem(parent);
	pNewItem->m_eType = MMenuModelItem::enuMenuOption;

	pNewItem->m_sText =  MHelpFuncs::getAttributeText(&currItemNode, IHM_CFG_VAL_MENU_FILE_LABEL_ATTRIBUTE);
	pNewItem->m_sActionType =   MHelpFuncs::getAttributeText(&currItemNode, IHM_CFG_VAL_MENU_FILE_ACTION_ATTRIBUTE);
	pNewItem->m_sActionParams =   MHelpFuncs::getAttributeText(&currItemNode, IHM_CFG_VAL_MENU_FILE_ACTION_PARAMS_ATTRIBUTE);
	pNewItem->m_sToolTip =   MHelpFuncs::getAttributeText(&currItemNode, IHM_CFG_VAL_MENU_FILE_TOOLTIP_ATTRIBUTE);
	pNewItem->m_sIcon =   MHelpFuncs::getAttributeText(&currItemNode, IHM_CFG_VAL_MENU_FILE_ICON_ATTRIBUTE);

	QString sEnabled =  MHelpFuncs::getAttributeText(&currItemNode, IHM_CFG_VAL_MENU_FILE_ENABLED_ATTRIBUTE);
	pNewItem->m_bEnabled = (sEnabled!="0")?true:false;

	QDomElement elSubMenu = currItemNode.firstChildElement(IHM_CFG_VAL_MENU_FILE_SUBMENU_ELEMENT);

	if(!elSubMenu.isNull())
	{
		pNewItem->m_sMenuID =   MHelpFuncs::getAttributeText(&elSubMenu, IHM_CFG_VAL_MENU_FILE_SUBMENU_ID_ATTRIBUTE);

		QDomNodeList lstItems = elSubMenu.childNodes();
		QDomNode currSubItemNode;
		
		int i=0;
		
		for(i=0;i<lstItems.count();i++)
		{
			currSubItemNode = lstItems.at(i); 
			
			if(currSubItemNode.nodeName()==IHM_CFG_VAL_MENU_FILE_ITEM_ELEMENT)
			{
				MMenuModelItem * pNewSubItem = recreateMenuItem(currSubItemNode, pNewItem);
				
				if(pNewSubItem!=NULL)
				{
					pNewItem->appendChild(pNewSubItem);				
					pNewItem->m_eType = MMenuModelItem::enuMenu;
				}

			}
		}	
	}

	return pNewItem;
}


bool MHMenuFileLoader::updateMenuItem(QDomNode currItemNode, MMenuModelItem * currentItem)
{
	
	currentItem->m_sText =  MHelpFuncs::getAttributeText(&currItemNode, IHM_CFG_VAL_MENU_FILE_LABEL_ATTRIBUTE);
	currentItem->m_sActionType =   MHelpFuncs::getAttributeText(&currItemNode, IHM_CFG_VAL_MENU_FILE_ACTION_ATTRIBUTE);
	currentItem->m_sActionParams =   MHelpFuncs::getAttributeText(&currItemNode, IHM_CFG_VAL_MENU_FILE_ACTION_PARAMS_ATTRIBUTE);

	QString sEnabled =  MHelpFuncs::getAttributeText(&currItemNode, IHM_CFG_VAL_MENU_FILE_ENABLED_ATTRIBUTE);
	currentItem->m_bEnabled = (sEnabled!="0")?true:false;

	QDomElement elSubMenu = currItemNode.firstChildElement(IHM_CFG_VAL_MENU_FILE_SUBMENU_ELEMENT);

	if(!elSubMenu.isNull())
	{
		currentItem->m_sMenuID =   MHelpFuncs::getAttributeText(&elSubMenu, IHM_CFG_VAL_MENU_FILE_SUBMENU_ID_ATTRIBUTE);

		QDomNodeList lstItems = elSubMenu.childNodes();
		
		QList <MMenuModelItem *> * pLst = currentItem->getChildItemsList();

		int iNumItems = lstItems.count();

		if(iNumItems == 0 && pLst==NULL)
			return true;
		
		if(pLst==NULL || pLst->size()!=lstItems.count())
				return false; //invalid menu structure for update (recreate function have to be used)
		
		for(int i=0;i<lstItems.count();i++)
		{
			if(lstItems.at(i).nodeName()!=IHM_CFG_VAL_MENU_FILE_ITEM_ELEMENT)
						return false;

			if(!updateMenuItem(lstItems.at(i), pLst->at(i)))
				return false;
		}	
	}


	return true;
}

bool MHMenuFileLoader::OpenXMLFile(QString sDataFilePath)
{
	bool bRet = false;

	QFile file;
	file.setFileName(sDataFilePath);
	
	if(file.open(QFile::ReadOnly))
	{
		QString sFileContent;
		QTextStream txtStream;
		txtStream.setDevice(&file);
		txtStream.setEncoding(QStringConverter::Utf8);
		
		sFileContent = txtStream.readAll(); 
			
		bRet = OpenXMLBuffer(sFileContent);
	}
	else
	{
		m_sLastError = QString( "MHMenuFileLoader::OpenXMLFile: Unable to open file %1")
									.arg(sDataFilePath);
	}

	return bRet;
}



bool MHMenuFileLoader::OpenXMLBuffer(QString sData)
{
	bool bRet = false;
	QString errorStr;
	int errorLine;
	int errorColumn;
	
	bRet = m_xmlData.setContent(sData, true, &errorStr, &errorLine, &errorColumn);

	if(bRet)
	{
		QDomElement root = m_xmlData.documentElement();
		if(root.tagName() != IHM_CFG_VAL_MENU_FILE_ROOT_ELEMENT)
		{
			m_sLastError = QString( "MHMenuFileView::OpenXMLBuffer: Invalid root element!");
		}
		else
		{
			bRet = true;
		}

	}
	else
	{
		QString sMsg = QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
		m_sLastError = QString( "MHMenuFileView::OpenXMLBuffer: %1")
									.arg(sMsg);
	}

	return bRet;
}
