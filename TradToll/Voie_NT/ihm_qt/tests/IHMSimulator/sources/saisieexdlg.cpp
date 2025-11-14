#include <QtGui>
#include <QListwidget>
#include <QSettings>

#include "constants.h"
#include "SaisieExDlg.h"

SaisieExDlg::SaisieExDlg(QWidget *parent, QString sIHMMailbox)
	: IHMSimuDld(parent, DLG_AFF, sIHMMailbox)
{
    ui.setupUi(this);

	if(OpenXMLConfig())
	{
		ReadXMLData();
		ReadRegValues();
	}
}

bool SaisieExDlg::ReadXMLData()
{
	bool bRet = true;

	return bRet;
}

bool SaisieExDlg::ReadRegValues()
{
	bool bRet = true;
	QStringList slValues;
	
	QSettings regValues(QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\IHM\\%1\\Input").arg(m_szMailbox), QSettings::NativeFormat);
	slValues = regValues.allKeys();
	for(int i = 0; i < slValues.size(); i++)
	{
		if(!slValues.at(i).contains("/"))
			ui.comboBoxType->addItem(slValues.at(i), NULL);// regValues.value(slValues.at(i), NULL).toString()
	}
	
	return bRet;
}

void SaisieExDlg::accept()
{
	m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_DEMANDE);
	m_pAction->setProperty(_PROP_DIALOG_ID, ui.comboBoxType->itemText(ui.comboBoxType->currentIndex()));
	m_pAction->setProperty(_PROP_IS_FILE, (ui.checkBoxIsFile->isChecked())? 1 : 0);
	m_pAction->setProperty(_PROP_VALUE, ui.textEditXML->toPlainText());
	
	m_pAction->activate(QAction::Trigger);
	
	QDialog::accept();
}

void SaisieExDlg::reject()
{
	QDialog::reject();
}

void SaisieExDlg::spinChanged(int r)
{
}
