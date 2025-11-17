#include <QtGui>
#include <QListwidget>
#include <QSettings>
#include "Ui_config.h"

#include "constants.h"
#include "config.h"


ConfigDlg::ConfigDlg(QWidget *parent, int iType, QString szMailbox)
	: IHMSimuDld(parent, DLG_CONF, szMailbox)
{
    ui->setupUi(this);

	m_iSubType = iType;

	m_pAction = NULL;

	if(OpenXMLConfig())
	{
		ReadXMLData();
		ReadRegValues();
	}
}

bool ConfigDlg::ReadXMLData()
{
	bool bRet = true;
	QString sSubType;
	switch(m_iSubType)
	{
	case CONF_GET:
		break;
	case CONF_SET:
		{
			QDomElement child = m_xmlConfig.documentElement().firstChildElement(XML_CONFIG);
			
			while(!child.isNull())
			{
				if(child.attribute("TYPE") == "TYPE")
				{
					ui->comboBoxLaneType->addItem(child.text());
				}
				else if(child.attribute("TYPE") == "LANGUAGE")
				{
					ui->comboBoxLanguage->addItem(child.text());
				}
				else if(child.attribute("TYPE") == "SHIFT")
				{
					ui->comboBoxTypeOfShift->addItem(child.text());
				}
				child = child.nextSiblingElement(XML_CONFIG);
			}
		}
		break;
	default:
		bRet = false;
	}

	
	return bRet;
}

bool ConfigDlg::ReadRegValues()
{
	bool bRet = true;

	switch(m_iSubType)
	{
	case CONF_GET:
		{
			m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_GET);
			m_pAction->activate(QAction::Trigger);
		}
		break;
	case CONF_SET:
		{
			QSettings regValuesIHM(QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\IHM\\%1").arg(m_szMailbox), QSettings::NativeFormat);
			ui->comboBoxLaneType->setEditText(regValuesIHM.value("DefaultLaneType", NULL).toString());
			ui->comboBoxLanguage->setEditText(regValuesIHM.value("DefaultLanguage", NULL).toString());
			ui->comboBoxTypeOfShift->setEditText(regValuesIHM.value("DefaultShiftType", NULL).toString());
			
			QSettings regValuesProject("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\LaneController\\Project", QSettings::NativeFormat);
			ui->lineEditProject->setText(regValuesProject.value("Name", NULL).toString());
			ui->lineEditVersion->setText(regValuesProject.value("Release", NULL).toString());
			
		}
		break;
	default:
		break;
	}
	
	return bRet;
}

void ConfigDlg::accept()
{
	switch(m_iSubType)
	{
	case CONF_GET:
		{
		}
		break;
	case CONF_SET:
		{
			m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_SET);
			m_pAction->setProperty(_PROP_PROJECT, ui->lineEditProject->text());
			m_pAction->setProperty(_PROP_VERSION, ui->lineEditVersion->text());
			m_pAction->setProperty(_PROP_LANGUAGE, ui->comboBoxLanguage->currentText());
			m_pAction->setProperty(_PROP_TYPE_OF_LANE, ui->comboBoxLaneType->currentText());
			m_pAction->setProperty(_PROP_TYPE_OF_SHIFT, ui->comboBoxTypeOfShift->currentText());
			m_pAction->activate(QAction::Trigger);
			
			QTime dieTime = QTime::currentTime().addMSecs(100);
			while( QTime::currentTime() < dieTime )
				;
		}
		break;
	default:
		break;
	}

	QDialog::accept();
}

void ConfigDlg::reject()
{
	QDialog::reject();
}