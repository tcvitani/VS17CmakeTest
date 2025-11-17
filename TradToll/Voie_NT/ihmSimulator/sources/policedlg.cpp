#include <QtGui>
#include <QListwidget>
#include <QSettings>

#include "constants.h"
#include "policedlg.h"

PoliceDlg::PoliceDlg(QWidget *parent, QString sIHMMailbox)
	: IHMSimuDld(parent, DLG_AFF, sIHMMailbox)
{
    ui.setupUi(this);

	if(OpenXMLConfig())
	{
		ReadXMLData();
		ReadRegValues();
	}
}

bool PoliceDlg::ReadXMLData()
{
	bool bRet = true;

	QDomElement child = m_xmlConfig.documentElement().firstChildElement("POLICE");
	
	while(!child.isNull())
	{
		ui.comboBoxTypeObj->addItem(child.text(), child.attribute("ID"));
		
		child = child.nextSiblingElement("POLICE");
	}

	return bRet;
}

bool PoliceDlg::ReadRegValues()
{
	return true;
}

void PoliceDlg::accept()
{
	m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_SET);
	m_pAction->setProperty(_PROP_OBJECT, ui.comboBoxTypeObj->itemData(ui.comboBoxTypeObj->currentIndex()));
	m_pAction->setProperty(_PROP_VALUE, ui.lineEditChaine->text());
	m_pAction->setProperty(_PROP_PROPERTY, ui.spinBoxProperty->text());
	m_pAction->activate(QAction::Trigger);
	
	QDialog::accept();
}

void PoliceDlg::reject()
{
	QDialog::reject();
}