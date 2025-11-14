
#ifndef MTAB_VIEW_H
#define MTAB_VIEW_H

#include <QTabWidget>


class MTabView:public QTabWidget
{
	Q_OBJECT
public:
	MTabView(QWidget * parent = 0);
	~MTabView();
	
	void init(QString sActionId);
	QTabBar *getTabBar(){ return m_pTab; };
signals:
	void clicked(QString,int);
	
protected:
	bool eventFilter(QObject *obj, QEvent *event);
	QString m_sActionId;
	QTabBar * m_pTab;
};



#endif

