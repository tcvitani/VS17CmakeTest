#include <QtGui>
#include <QListwidget>
#include <QSettings>

#include "constants.h"
#include "saisiedlg.h"

SaisieDlg::SaisieDlg(QWidget *parent, QString sIHMMailbox)
	: IHMSimuDld(parent, DLG_AFF, sIHMMailbox)
{
    ui.setupUi(this);

	if(OpenXMLConfig())
	{
		ReadXMLData();
		ReadRegValues();
	}
}

bool SaisieDlg::ReadXMLData()
{
	bool bRet = true;

	QDomElement child = m_xmlConfig.documentElement().firstChildElement("SAISIE");
	
	while(!child.isNull())
	{
		ui.comboBoxType->addItem(child.attribute("DLG_TYPE"), child.attribute("ID"));
		child = child.nextSiblingElement("SAISIE");
	}
	
	return bRet;
}

bool SaisieDlg::ReadRegValues()
{
	bool bRet = true;
	QStringList slValues;
	
	QSettings regValues(QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\IHM\\%1\\Input").arg(m_szMailbox), QSettings::NativeFormat);
	slValues = regValues.allKeys();
	for(int i = 0; i < slValues.size(); i++)
	{
		if(!slValues.at(i).contains("/"))
			ui.comboBoxName->addItem(slValues.at(i), NULL);// regValues.value(slValues.at(i), NULL).toString()
	}
	
	return bRet;
}

void SaisieDlg::accept()
{
	//ime dlg             no labela                  
	//saisie_choix_devise|1|3~3.Denmark Kroner      |

	QString szDefinition;
	szDefinition.append(QString("%1|%2").arg(ui.comboBoxName->currentText()).arg(ui.spinBoxNoControls->text()));
	szDefinition.append(ui.textEditControls->toPlainText());

	m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_DEMANDE);
	m_pAction->setProperty(_PROP_DEFINITION, szDefinition);
	m_pAction->setProperty(_PROP_SAISIE, ui.comboBoxType->itemData(ui.comboBoxType->currentIndex()));
	m_pAction->setProperty(_PROP_BUTTONS, ui.spinBoxButtons->text());
	
	m_pAction->activate(QAction::Trigger);
	
	QDialog::accept();
}

void SaisieDlg::reject()
{
	QDialog::reject();
}

void SaisieDlg::spinChanged(int r)
{
}
