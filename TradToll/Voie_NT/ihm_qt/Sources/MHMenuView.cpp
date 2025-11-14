#include <QVBoxLayout>
#include <QFile>
#include <QMenuBar>
#include <QIcon>
#include <QString>
#include <QPushButton>

#include "MHMenuView.h"
#include "MMenuModelItem.h"
#include "MHMenuFileLoader.h"
#include "MIhmConfigGeneral.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}


MIHMMenuAction::MIHMMenuAction(QObject * parent, const QString & text,  QString sAction, QString sParams)
	:QWidgetAction(parent)
{

	this->setText(text);
	m_sAction = sAction;
	m_sParams = sParams;
	m_bEnabled = true;
	connect(this, SIGNAL(triggered(bool)), this, SLOT(onTriggered(bool)),Qt::DirectConnection);

}

MIHMMenuAction::~MIHMMenuAction()
{
}


QWidget * MIHMMenuAction::createWidget(QWidget * parent)
{
	Q_UNUSED(parent)

	return NULL;
}

void MIHMMenuAction::setEnabled(bool bEnabled)
{
	m_bEnabled = bEnabled;

	QWidgetAction::setEnabled(bEnabled);
}

void MIHMMenuAction::onTriggered(bool bChecked)
{
	Q_UNUSED(bChecked)
	
	if (m_bEnabled)
		emit execAction(m_sAction, m_sParams);
}
	

//----------------



MHMenuView::MHMenuView(QWidget *parent)
	:QMenuBar(parent)
{

}

MHMenuView::~MHMenuView()
{
	clearAndDelete();
}


void MHMenuView::clearAndDelete()
{
	this->setActiveAction(NULL);
	this->clear();

	while(!m_lstCreatedSubObjects.isEmpty())
		delete m_lstCreatedSubObjects.takeLast();

}


void MHMenuView::updateMenu(QList <MMenuModelItem*> *lstItems, bool bRecreate)
{
	TRACE_D(QString( "MHMenuView::updateMenu: bRecreate:%1").arg(bRecreate));

	if(bRecreate)
	{
		clearAndDelete();
	}

	updateMenuView(this, lstItems, bRecreate);

	TRACE_D(QString( "MHMenuView::updateMenu...Exit OK!"));

}	

void MHMenuView::collapseAll(QList <MMenuModelItem*> *lstItems)
{
	MMenuModelItem * pItem;

	for(int i = 0; i<lstItems->size();i++)
	{
		pItem = lstItems->at(i);

		if(pItem->m_eType == MMenuModelItem::enumMenuItemType::enuMenu)
		{
			QMenu *pNewMenu = NULL;
			pNewMenu = pItem->getCreatedMenu();
			
			if(pNewMenu != NULL)
			{
				pNewMenu->close();
			}
		}
	}
}




void MHMenuView::updateMenuView(QWidget *pMenu, QList <MMenuModelItem*> * lstItems, bool bRecreate)
{
	MMenuModelItem * pItem = NULL;
	MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	QString sTextToShow;

	for(int i = 0; i<lstItems->size();i++)
	{
		pItem = lstItems->at(i);
	
		if(pItem->m_eType == MMenuModelItem::enumMenuItemType::enuMenu)
		{
			QMenu *pNewMenu = NULL;

			if(bRecreate)
			{
				pLang->getLabelTranslation(pItem->m_sText, MIhmLanguages::enuTranslTargetDesktop, sTextToShow);
				pNewMenu = new QMenu(sTextToShow, pMenu);

				if(pItem->m_sToolTip!="")
				{
					pLang->getLabelTranslation(pItem->m_sToolTip, MIhmLanguages::enuTranslTargetDesktop, sTextToShow);
					pNewMenu->setToolTip(sTextToShow);
				}

				if(pItem->m_sIcon!="")
				{
					QString sIconPath = MIhmConfigGeneral::getCfg()->getDskConfigImages()->getImageFullPath(pItem->m_sIcon);
					
					if(sIconPath!="")
					{
						QIcon oIcon(sIconPath);
						
						if(!oIcon.isNull())
							pNewMenu->setIcon(oIcon);
					}
				}

				if(pMenu == this)
					((QMenuBar*)pMenu)->addMenu(pNewMenu);
				else
					((QMenu*)pMenu)->addMenu(pNewMenu);

				pItem->setCreatedMenu(pNewMenu); //to be able to update without recreation

				m_lstCreatedSubObjects.append(pNewMenu);//to be able to delete it when needed

			}
			else
			{
				pNewMenu = pItem->getCreatedMenu();
			}

			if(pNewMenu != NULL)
			{
				pNewMenu->setEnabled(pItem->m_bEnabled);
				updateMenuView(pNewMenu, pItem->getChildItemsList(), bRecreate);
			}
			else
			{
				TRACE_W(QString( "MHMenuView::updateMenuView: Unable to find child QMenu [%1] of object[%2]!")
									.arg(pItem->m_sText)
									.arg(pMenu->objectName()));
				
			}
		}
		else
		{
			MIHMMenuAction *pNewAction = NULL;
			
			if(bRecreate)
			{
				pLang->getLabelTranslation(pItem->m_sText, MIhmLanguages::enuTranslTargetDesktop, sTextToShow);
				pNewAction = new MIHMMenuAction(pMenu, sTextToShow, pItem->m_sActionType, pItem->m_sActionParams);
				
				if(pItem->m_sToolTip!="")
				{
					pLang->getLabelTranslation(pItem->m_sToolTip, MIhmLanguages::enuTranslTargetDesktop, sTextToShow);
					pNewAction->setToolTip(sTextToShow);
				}
				
				if(pItem->m_sIcon!="")
				{
					QString sIconPath = MIhmConfigGeneral::getCfg()->getDskConfigImages()->getImageFullPath(pItem->m_sIcon);
					
					if(sIconPath!="")
					{
						QIcon oIcon(sIconPath);
						
						if(!oIcon.isNull())
							pNewAction->setIcon(oIcon);
					}
				}
				
				connect(pNewAction, SIGNAL(execAction(QString, QString)), this, SLOT(onExecAction(QString, QString)));
				
				if(pMenu == this)
					((QMenuBar*)pMenu)->addAction((QAction*)pNewAction);
				else
					((QMenu*)pMenu)->addAction((QAction*)pNewAction);
				
				pItem->setCreatedMenuAction(pNewAction); //to be able to update without recreation
				
				m_lstCreatedSubObjects.append(pNewAction);//to be able to delete it when needed
			}
			else
			{
				pNewAction = pItem->getCreatedMenuAction();
			}
			
			if(pNewAction != NULL)
			{

				pNewAction->setEnabled(pItem->m_bEnabled);
			}
			else
			{
				TRACE_W(QString( "MHMenuView::updateMenuView: Unable to find child MIHMMenuAction [%1] of object[%2]!")
									.arg(pItem->m_sText)
									.arg(pMenu->objectName()));
				
			}
				
		}
	}

}

void MHMenuView::onExecAction(QString sAction, QString sParams)
{
	emit action(sAction, sParams);
}

