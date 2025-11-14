// IHMSimuDld.h: interface for the IHMSimuDld class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IHMSIMUDLD_H__65BD381D_2A33_4C74_BC98_561CBC8866AD__INCLUDED_)
#define AFX_IHMSIMUDLD_H__65BD381D_2A33_4C74_BC98_561CBC8866AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <QDialog>
#include <QString>
#include <QDomDocument>
#include <QFile>
#include "ihm.h"

class IHMSimuDld : public QDialog  
{
public:
	IHMSimuDld(QWidget *parent, int iType, QString szMailbox);
	virtual ~IHMSimuDld();

	enum
	{
		DLG_AFF,
		DLG_CONF,
		DLG_TOUCHE,
		DLG_SASIE,
		DLG_POLICE,
		DLG_DETECTION,
		DLG_INDICATEUR,
		DLG_SCRIPT,
		DLG_VIDEO
	};

	void setAction(QAction * pAction);

protected:
	QString m_szXMLFile;
	QString m_szRootTag;
	QString m_szMailbox;
	QAction * m_pAction;
	QDomDocument m_xmlConfig;
	
	bool OpenXMLConfig();
	virtual bool ReadXMLData();
	
private:
	int m_iType;
	QFile m_fConfFile;

};

#endif // !defined(AFX_IHMSIMUDLD_H__65BD381D_2A33_4C74_BC98_561CBC8866AD__INCLUDED_)
