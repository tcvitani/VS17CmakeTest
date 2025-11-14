
#include <QMenu.h>
#include "MHMenuView.h"
#include "MMenuModelItem.h"


MMenuModelItem::MMenuModelItem(MMenuModelItem * parent):m_pCreatedMenu(NULL), m_pCreatedMenuAction(NULL)
{
	m_parentMenu = parent;
	m_eType = enuMenu;
}

MMenuModelItem::~MMenuModelItem()
{
	while(!m_lstChildItems.isEmpty())
		delete m_lstChildItems.takeFirst();
	
}

	
void MMenuModelItem::appendChild(MMenuModelItem *pChild)
{
	m_lstChildItems.append(pChild);
}



QList <MMenuModelItem *> * MMenuModelItem::getChildItemsList()
{
	if(m_eType == enuMenu)
	{
		return &m_lstChildItems;
	}
	else
		return NULL;
}
