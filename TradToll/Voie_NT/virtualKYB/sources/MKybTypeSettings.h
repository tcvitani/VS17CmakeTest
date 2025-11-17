#ifndef MKYB_Type_Settings_H
#define MKYB_Type_Settings_H

#include <QString>
#include <QList>
#include <QPoint>


struct KybEvent{
	quint8 bVk; 
	quint32 dwFlags;
};

class MKybKeyCfg
{
public:
	MKybKeyCfg(){};
	~MKybKeyCfg();

	MKybKeyCfg& operator=(const MKybKeyCfg& right);

	bool appendKeyEventsString(QByteArray baCfgLine);
	
	QString getWidgetName(){return m_sWidgetName;};
	void setWidgetName(QString sName){m_sWidgetName = sName;};

	QString m_sWidgetName;	
	
	QList <KybEvent*> m_lstKybEvents;
};


class MKybTypeSettings
{
	
public:
	MKybTypeSettings();
	~MKybTypeSettings();
	
	bool loadKeybSettings(QString sCfgID, QString sFileName);
	QString getCfgId(){return m_sCfgID;};
	QString getCfgFileName(){return m_sCfgFile;};
	QString getUIFileNamePath(){return m_sUIFileNamePath;};
	QList <MKybKeyCfg*> * getKybKeysList(){return &m_lstKybKeys;};

	QPoint &getInitialPos(){return m_ptInitialPos;};
	bool showTitleBar(){return m_bShowTitleBar;}
private:
	void clearData();
	bool processCfgLine(QByteArray baLine);
	QList <MKybKeyCfg*> m_lstKybKeys;

	QString m_sCfgID;
	QString m_sCfgFile;
	QString m_sUIFileNamePath;
	QPoint m_ptInitialPos;
	bool m_bShowTitleBar;
};


#endif


