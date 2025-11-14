#include <QtGui>
#include <QListwidget>
#include <QSettings>
#include <ihm.h>

#include "constants.h"
#include "videodlg.h"

VideoDlg::VideoDlg(QWidget *parent, QString sIHMMailbox)
	: IHMSimuDld(parent, DLG_VIDEO, sIHMMailbox)
{
    ui.setupUi(this);

	QObject::connect(ui.comboBoxCommand, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdChanged(int)));
	
	if(OpenXMLConfig())
	{
		ReadXMLData();
		ReadRegValues();
	}
	ui.spinBoxSource->setEnabled(false);
	ui.lineEditValue->setEnabled(false);
}

bool VideoDlg::ReadXMLData()
{
	bool bRet = true;

	QDomElement child = m_xmlConfig.documentElement().firstChildElement(XML_VIDEO);

	while(!child.isNull())
	{
		if(child.attribute("TYPE") == "COMMAND")
		{
			ui.comboBoxCommand->addItem(child.text(), child.attribute("ID"));
		}
		child = child.nextSiblingElement(XML_VIDEO);
		
	}
	
	return bRet;
}

bool VideoDlg::ReadRegValues()
{
	return true;
}

void VideoDlg::accept()
{
	m_pAction->setProperty(_PROP_TYPE, "DEMANDE");
	m_pAction->setProperty(_PROP_VIDEO, ui.spinBoxVideoView->text());
	m_pAction->setProperty(_PROP_SOURCE, ui.spinBoxSource->text());
	m_pAction->setProperty(_PROP_COMMAND, ui.comboBoxCommand->itemData(ui.comboBoxCommand->currentIndex()));
	m_pAction->setProperty(_PROP_VALUE, ui.lineEditValue->text());
	m_pAction->activate(QAction::Trigger);
	
	QDialog::accept();
}

void VideoDlg::reject()
{
	QDialog::reject();
}

void VideoDlg::onCmdChanged(int iCmd)
{
	switch(ui.comboBoxCommand->itemData(ui.comboBoxCommand->currentIndex()).toInt())
	{
	case IHM_VIDEO_ON:
		ui.spinBoxSource->setEnabled(false);
		ui.lineEditValue->setEnabled(false);
		break;
	case IHM_VIDEO_OFF:
		ui.spinBoxSource->setEnabled(false);
		ui.lineEditValue->setEnabled(false);
		break;
	case IHM_VIDEO_FREEZE:
		ui.spinBoxSource->setEnabled(false);
		ui.lineEditValue->setEnabled(false);
		break;
	case IHM_VIDEO_UNFREEZE:
		ui.spinBoxSource->setEnabled(false);
		ui.lineEditValue->setEnabled(false);
		break;
	case IHM_VIDEO_ZOOM_IN:
		ui.spinBoxSource->setEnabled(false);
		ui.lineEditValue->setEnabled(false);
		break;
	case IHM_VIDEO_ZOOM_OUT:
		ui.spinBoxSource->setEnabled(false);
		ui.lineEditValue->setEnabled(false);
		break;
	case IHM_VIDEO_ZOOM:
		ui.spinBoxSource->setEnabled(false);
		ui.lineEditValue->setEnabled(true);
		break;
	case IHM_VIDEO_SELECT_SRC:
		ui.spinBoxSource->setEnabled(true);
		ui.lineEditValue->setEnabled(false);
		break;
	case IHM_VIDEO_SAVE_BMP:
		ui.spinBoxSource->setEnabled(false);
		ui.lineEditValue->setEnabled(true);
		break;
	case IHM_VIDEO_SAVE_JPG:
		ui.spinBoxSource->setEnabled(false);
		ui.lineEditValue->setEnabled(true);
		break;
	default:
		break;
	}
}