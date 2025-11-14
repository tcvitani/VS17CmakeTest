#ifndef M_MENU_MODEL_ITEM_H
#define M_MENU_MODEL_ITEM_H

#include <QString>
#include <QList>

class QMenu;
class MIHMMenuAction;

class MMenuModelItem
{
public:
	MMenuModelItem(MMenuModelItem * parent = 0);
	~MMenuModelItem();

	enum enumMenuItemType{
		enuMenu,
		enuMenuOption,
	};
	
	void appendChild(MMenuModelItem * pChild);

	MMenuModelItem * getParentMenu(){return m_parentMenu;};
	QList <MMenuModelItem *> * getChildItemsList();
	

	enumMenuItemType m_eType;
	QString m_sIcon;
	QString m_sToolTip;
	QString m_sText;
	QString m_sActionType;
	QString m_sActionParams;
	QString m_sMenuID;
	bool m_bEnabled;
	
	void setCreatedMenu(QMenu *pCreatedMenu)
				{m_pCreatedMenu = pCreatedMenu;};
	QMenu * getCreatedMenu()
				{return m_pCreatedMenu;};

	void setCreatedMenuAction(MIHMMenuAction *pCreatedMenuAction)
				{m_pCreatedMenuAction = pCreatedMenuAction;};
	MIHMMenuAction * getCreatedMenuAction()
				{return m_pCreatedMenuAction;};
private:

	MMenuModelItem * m_parentMenu;

	QList <MMenuModelItem *> m_lstChildItems;

	QMenu *m_pCreatedMenu;
	MIHMMenuAction * m_pCreatedMenuAction;
};




#endif

