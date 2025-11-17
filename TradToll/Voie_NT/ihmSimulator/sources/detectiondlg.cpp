#include <QtGui>
#include <QListwidget>
#include <QSettings>

#include "constants.h"
#include "DetectionDlg.h"

DetectionDlg::DetectionDlg(QWidget *parent, QString szMailbox)
	: IHMSimuDld(parent, DLG_AFF, szMailbox)
{
    ui.setupUi(this);

	if(OpenXMLConfig())
	{
		ReadXMLData();
		ReadRegValues();
	}
}

bool DetectionDlg::ReadXMLData()
{
	bool bRet = true;

	return bRet;
}

bool DetectionDlg::ReadRegValues()
{
	bool bRet = true;
	QStringList slValues;
	QSettings regValues(QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\IHM\\%1\\StringDetection").arg(m_szMailbox), QSettings::NativeFormat);
	slValues = regValues.allKeys();
	for(int i = 0; i < slValues.size(); i++)
	{
		ui.comboBoxRegKey->addItem(slValues.at(i), NULL);
		ui.comboBoxRegValue->addItem(regValues.value(slValues.at(i), NULL).toString(), NULL);
	}

	return bRet;
}

void DetectionDlg::accept()
{
	m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_DEBUT);
	m_pAction->setProperty(_PROP_COUPLE_ID, ui.comboBoxRegKey->currentText());
	m_pAction->setProperty(_PROP_SAISIE, ui.comboBoxRegValue->currentText());
	m_pAction->setProperty(_PROP_PREPOSTAMBULES, ui.spinBoxPrePostambules->text());
	m_pAction->activate(QAction::Trigger);
	
	QDialog::accept();
}

void DetectionDlg::reject()
{
	QDialog::reject();
}