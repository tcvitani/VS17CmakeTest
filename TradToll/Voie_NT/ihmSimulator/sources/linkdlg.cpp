#include <QtGui>
#include <QListwidget>
#include <QSettings>

#include "constants.h"
#include "linkdlg.h"

LinkDlg::LinkDlg(QWidget *parent, QString szMailbox)
	: IHMSimuDld(parent, DLG_AFF, szMailbox)
{
    ui.setupUi(this);

	if(OpenXMLConfig())
	{
		ReadXMLData();
		ReadRegValues();
	}
}

bool LinkDlg::ReadXMLData()
{
	bool bRet = true;

	ui.spinBoxDialogID->setMinimum(0);
	ui.spinBoxDialogID->setMaximum(3);
	
	return bRet;
}

bool LinkDlg::ReadRegValues()
{
	bool bRet = true;

	return bRet;
}

void LinkDlg::accept()
{
	m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_SET);
	m_pAction->setProperty(_PROP_OBJECT, (int)IHM_OBJ_OpenLink);
	m_pAction->setProperty(_PROP_VALUE, ui.lineEditLink->text());
	m_pAction->setProperty(_PROP_DIALOG_ID, ui.spinBoxDialogID->text());
	
	m_pAction->activate(QAction::Trigger);
	
	QDialog::accept();
}

void LinkDlg::reject()
{
	QDialog::reject();
}

