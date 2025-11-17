#include <QtGui>
#include <QListwidget>
#include <QSettings>
#include "Ui_Affichage.h"

#include "constants.h"
#include "Affichage.h"
#include "SimuSendReciveThread.h"

AffichageDlg::AffichageDlg(QWidget *parent, int iType, QString szMailbox, QString szLanguage)
	: IHMSimuDld(parent, DLG_AFF, szMailbox)
{
    ui->setupUi(this);

	m_iSubType = iType;
	m_szLanguage = szLanguage;
	
	if(OpenXMLConfig())
	{
		ReadXMLData();
		ReadRegValues();
	}
}

bool AffichageDlg::ReadXMLData()
{
	bool bRet = true;
	QString sSubType;
	switch(m_iSubType)
	{
	case AFF_ICON:
		sSubType = _PROP_VAL_ICON;
		break;
	case AFF_LABEL:
		sSubType = _PROP_VAL_LABEL;
		break;
	case AFF_IMAGE:
		sSubType = _PROP_VAL_IMAGE;
		break;
	default:
		sSubType = "_ERROR_";
	}

	QDomElement child = m_xmlConfig.documentElement().firstChildElement(XML_AFFICHAGE);

	while(!child.isNull())
	{
		if(child.attribute("TYPE") == sSubType)
		{
			ui->comboBoxObject->addItem(child.text(), SimuSendReciveThread::getObjectIdFromString(QString(child.text())));//MMM child.attribute("ID"));
		}
		child = child.nextSiblingElement(XML_AFFICHAGE);
		
	}
	
	return bRet;
}

bool AffichageDlg::ReadRegValues()
{
	bool bRet = true;
	QStringList slValues;

	switch(m_iSubType)
	{
	case AFF_ICON:
		{
			QSettings regValues(QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\IHM\\%1\\Images").arg(m_szMailbox), QSettings::NativeFormat);
			slValues = regValues.allKeys();
			for(int i = 0; i < slValues.size(); i++)
			{
				ui->comboBoxRegKey->addItem(slValues.at(i).section("/", -1), NULL);// regValues.value(slValues.at(i), NULL).toString()
			}
		}
		break;
	case AFF_LABEL:
		{
			QSettings regValues(QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\LaneController\\Languages\\%1").arg(m_szLanguage), QSettings::NativeFormat);
			slValues = regValues.allKeys();
			for(int i = 0; i < slValues.size(); i++)
			{
				ui->comboBoxRegKey->addItem(slValues.at(i).section("/", -1), NULL);// regValues.value(slValues.at(i), NULL).toString()
			}
		}
		break;
	case AFF_IMAGE:
		{
			QSettings regValues(QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\IHM\\%1\\Images\\Desktop").arg(m_szMailbox), QSettings::NativeFormat);
			slValues = regValues.allKeys();
			for(int i = 0; i < slValues.size(); i++)
			{
				if(slValues.at(i).section("/", -1) == "ImagesRoot")
					m_szImageRoot = regValues.value(slValues.at(i), NULL).toString();
				else
					ui->comboBoxRegKey->addItem(regValues.value(slValues.at(i), NULL).toString(), NULL);// slValues.at(i).section("/", -1)
			}
		}
		break;
	default:
		break;
	}
	
	return bRet;
}

void AffichageDlg::accept()
{
	switch(m_iSubType)
	{
	case AFF_ICON:
		{
			m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_SET);
			m_pAction->setProperty(_PROP_OBJECT, ui->comboBoxObject->itemData(ui->comboBoxObject->currentIndex()));
			m_pAction->setProperty(_PROP_VALUE, ui->comboBoxRegKey->currentText());// itemData(ui->comboBoxRegKey->currentIndex()));
			m_pAction->setProperty(_PROP_PROPERTY, ui->spinBoxProperty->text());
			m_pAction->activate(QAction::Trigger);
			
			QTime dieTime = QTime::currentTime().addMSecs(100);
			while( QTime::currentTime() < dieTime )
				;
		}
		break;
	case AFF_LABEL:
		{
			m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_SET);
			m_pAction->setProperty(_PROP_OBJECT, ui->comboBoxObject->itemData(ui->comboBoxObject->currentIndex()));
			m_pAction->setProperty(_PROP_VALUE, ui->comboBoxRegKey->currentText());// itemData(ui->comboBoxRegKey->currentIndex()));
			m_pAction->setProperty(_PROP_PROPERTY, ui->spinBoxProperty->text());
			m_pAction->activate(QAction::Trigger);
			
			QTime dieTime = QTime::currentTime().addMSecs(100);
			while( QTime::currentTime() < dieTime )
				;
		}
		break;
	case AFF_IMAGE:
		{
			m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_SET);
			m_pAction->setProperty(_PROP_OBJECT, ui->comboBoxObject->itemData(ui->comboBoxObject->currentIndex()));
			m_pAction->setProperty(_PROP_VALUE, QString("%1\\%2").arg(m_szImageRoot).arg(ui->comboBoxRegKey->currentText()));// itemData(ui->comboBoxRegKey->currentIndex()));
			m_pAction->setProperty(_PROP_PROPERTY, ui->spinBoxProperty->text());
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

void AffichageDlg::reject()
{
	QDialog::reject();
}