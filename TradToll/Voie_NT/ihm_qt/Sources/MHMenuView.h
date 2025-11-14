
#ifndef MHMENU_VIEW_H
#define MHMENU_VIEW_H

#include <QMenuBar>
#include <QHBoxLayout>
#include <QWidgetAction>
#include "MHMenuFileLoader.h"


class MHMenuModelItem;


class MIHMMenuAction:public QWidgetAction
{
	Q_OBJECT
public:
	MIHMMenuAction(QObject * parent, const QString & text, QString sAction, QString sParams);
	~MIHMMenuAction();
	
	void setEnabled(bool bEnabled);

signals:
	void execAction(QString sAction, QString sParams);
		
public slots:
	void onTriggered(bool bChecked);

protected:
	virtual QWidget * createWidget(QWidget * parent);

	QString m_sAction;
	QString m_sParams;
	bool m_bEnabled;

};


class MHMenuView:public QMenuBar
{
	Q_OBJECT
public:
	MHMenuView(QWidget *parent=0);
	~MHMenuView();
	
	void updateMenu(QList <MMenuModelItem*> *lstItems, bool bRecreate);		
	void collapseAll(QList <MMenuModelItem*> *lstItems);
signals:
	void action(QString sAction, QString sParams);

public slots:
	void onExecAction(QString sAction, QString sParams);
	
protected:
	void clearAndDelete();
	void updateMenuView(QWidget *pMenu, QList <MMenuModelItem*> * lstItems, bool bRecreate);
	

	QList <QObject*> m_lstCreatedSubObjects;
};



#endif

