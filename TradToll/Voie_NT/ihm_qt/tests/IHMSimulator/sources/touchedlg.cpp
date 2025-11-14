#include <QtGui>
#include <QListwidget>
#include <QSettings>

#include "constants.h"
#include "touchedlg.h"

ToucheDlg::ToucheDlg(QWidget *parent, QString sIHMMailbox)
	: IHMSimuDld(parent, DLG_AFF, sIHMMailbox)
{
    ui.setupUi(this);

	if(OpenXMLConfig())
	{
		ReadXMLData();
		ReadRegValues();
	}
}

bool ToucheDlg::ReadXMLData()
{
	bool bRet = true;

	QDomElement child = m_xmlConfig.documentElement().firstChildElement(XML_TOUCHE);

	while(!child.isNull())
	{
		if(child.attribute("TYPE") == "ETAT")
		{
			ui.comboBoxEtat->addItem(child.text(), child.attribute("ID"));
		}
		child = child.nextSiblingElement(XML_TOUCHE);
		
	}
	
	return bRet;
}

bool ToucheDlg::ReadRegValues()
{
	return true;
}

void ToucheDlg::accept()
{
	m_pAction->setProperty(_PROP_TYPE, "DEMANDE");
	m_pAction->setProperty(_PROP_GROUPE, ui.lineNomGroupe->text());
	m_pAction->setProperty(_PROP_ASCII, ui.lineASCII->text());
	m_pAction->setProperty(_PROP_ETAT, ui.comboBoxEtat->itemData(ui.comboBoxEtat->currentIndex()));
	m_pAction->activate(QAction::Trigger);
	
	QDialog::accept();
}

void ToucheDlg::reject()
{
	QDialog::reject();
}