// IHMSimuDld.cpp: implementation of the IHMSimuDld class.
//
//////////////////////////////////////////////////////////////////////

#include "IHMSimuDld.h"
#include <QMessageBox>
#include <QDomElement>
#include "constants.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IHMSimuDld::IHMSimuDld(QWidget *parent, int iType, QString szMailbox)
	: QDialog(parent)
{
	m_iType = iType;
	m_szMailbox = szMailbox;
	m_szXMLFile = SIMU_CONF_FILE;
	m_szRootTag = SIMU_CONF_FILE_ROOT_TAG;
}

IHMSimuDld::~IHMSimuDld()
{
}

bool IHMSimuDld::OpenXMLConfig()
{
	bool bRet;
	QString errorStr;
	int errorLine;
	int errorColumn;

	m_fConfFile.setFileName(m_szXMLFile);
	m_fConfFile.open(QIODevice::ReadOnly);

	bRet = m_xmlConfig.setContent((QIODevice*)&m_fConfFile, true, &errorStr, &errorLine, &errorColumn);

	if(!bRet)
	{
		QMessageBox::information(window(), tr("XML Error"),	tr("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr));
	}
	else
	{
		QDomElement root = m_xmlConfig.documentElement();
		if(root.tagName() != m_szRootTag)
		{
			QMessageBox::information(window(), tr("XML Error"),	tr("XML not good!"));
			bRet = false;
		}
	}

	return bRet;
}

bool IHMSimuDld::ReadXMLData()
{
	return false;
}

void IHMSimuDld::setAction(QAction * pAction)
{
	m_pAction = pAction;
}

