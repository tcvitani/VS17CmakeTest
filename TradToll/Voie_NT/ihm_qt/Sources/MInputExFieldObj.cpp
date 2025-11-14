
#include <QHBoxLayout>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QValidator>

#include "MInputExFieldObj.h"
#include "MLineEdit.h"
#include "MHelpFuncs.h"
#include "MIhmConfigGeneral.h"
#include "MIhmComboBox.h"
#include "MIhmListWidget.h"
#include "MTracer.h"


MInputExFieldObj::MInputExFieldObj()
{
	m_bObjConnected = false;
	m_bUpdateToWidgetInProgress = false;
}

MInputExFieldObj::~MInputExFieldObj()
{

}

MInputExFieldObj * MInputExFieldObj::findFieldObject(QList <MInputExFieldObj*> *plst, QString sName)
{
	MInputExFieldObj *pRetObj = NULL;

	for (int i = 0; i < plst->size(); ++i) 
	{
		MInputExFieldObj *pCurrent = plst->at(i);
		if(pCurrent->getName() == sName)
		{
			pRetObj = pCurrent;
			break;
		}
	}

	return pRetObj;
}

void MInputExFieldObj::updateData(MInputExField* pData)
{
	if(m_fieldData.getName() == pData->getName() && m_fieldData.getType() == pData->getType())
	{
		m_fieldData = *pData;
		updateDataToWidget();
	}
}


MInputExFieldObj * MInputExFieldObj::createNewFieldObj(MInputExField * pFieldDef)
{
	MInputExFieldObj * pNewObj = NULL;

	if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_CHECK_BOX)
		pNewObj = new MFieldObjCheckBox();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_RADIO_BUTTON)
		pNewObj = new MFieldObjRadioButton();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_COMBO_BOX)
		pNewObj = new MFieldObjComboBox();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_LIST_WIDGET)
		pNewObj = new MFieldObjListWidget();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_LINE_EDIT)
		pNewObj = new MFieldObjLineEdit();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_HIDDEN_EDIT)
		pNewObj = new MFieldObjLineEdit();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT)
		pNewObj = new MFieldObjLineEditEx();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_TEXT_EDIT)
		pNewObj = new MFieldObjTextEdit();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_LABEL)
		pNewObj = new MFieldObjLabel();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_LABEL_LUHN_CHECK)
		pNewObj = new MFieldObjLuhnCheckLabel();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON_OK)
		pNewObj = new MFieldObjButton();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON_CANCEL)
		pNewObj = new MFieldObjButton();
	else if(pFieldDef->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON)
		pNewObj = new MFieldObjButton();
	else
	{
		TRACE_W(QString("MInputExFieldObj::createNewFieldObj: Unknown object type %1")
									.arg(pFieldDef->getType()));

	}

	return pNewObj;
}




QString MInputExFieldObj::getXMLValue()
{
	QString sXMLValue = m_fieldData.getXMLValue();

	return sXMLValue;
}



//-----------------------------------------------
bool MFieldObjCheckBox::connectToWidget(QWidget *parentForm, MInputExField* pData)
{
	m_fieldData = *pData;

	m_pCheckBox = this->findChild<QCheckBox*>(m_fieldData.getName());

	if(m_pCheckBox!=NULL)
	{
		m_bObjConnected = true;
		updateDataToWidget();
		connect(m_pCheckBox,SIGNAL(stateChanged(int)), this, SLOT(onDataChanged(int)));
	}
	else
	{
		TRACE_W(QString( "MFieldObjCheckBox::connectToWidget: Unable to find QCheckBox:%1")
									.arg(m_fieldData.getName()));
	}


	return m_bObjConnected;
}

void MFieldObjCheckBox::onDataChanged(int iState)
{
	//do not execute if widget is updated by module
	if(m_bUpdateToWidgetInProgress) 
		return;

	QString sOldValue = m_fieldData.getValue();
	Qt::CheckState eOldState = 
				((sOldValue=="0")||(sOldValue=="")||(sOldValue=="false"))?Qt::Unchecked:Qt::Checked;

	Qt::CheckState eNewState = m_pCheckBox->checkState();
	
	if(eOldState!=eNewState)
	{
		m_fieldData.setValue(getCurrentValue());
		emit dataChanged();
	}
}


void MFieldObjCheckBox::updateDataToWidget()
{
	if(m_pCheckBox==NULL)
		return;

	m_bUpdateToWidgetInProgress = true;

	QString sText =	m_fieldData.getText();

	MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	QString sTranslation;
	pLang->getLabelTranslation(sText, MIhmLanguages::enuTranslTargetDesktop, sTranslation);
	
	//Convert label special characters ...
	m_pCheckBox->setText(sTranslation);
	
	QString sValue = m_fieldData.getValue();
	Qt::CheckState eState = 
				((sValue=="0")||(sValue=="")||(sValue=="false"))?Qt::Unchecked:Qt::Checked;

	m_pCheckBox->setCheckState(eState);

	m_pCheckBox->setEnabled(!m_fieldData.isReadOnly());

	m_bUpdateToWidgetInProgress = false;
}

QString MFieldObjCheckBox::getCurrentValue()
{
	Qt::CheckState eNewState = m_pCheckBox->checkState();
	QString sValue = (eNewState == Qt::Checked)?"1":"0";

	return sValue;
}



QString MFieldObjCheckBox::getEncStrValue()
{
	QString sRes;

	if(m_pCheckBox!=NULL)
	{
		QString sState = getCurrentValue();

		sRes = QString("%1~%2").arg(m_fieldData.getName()).arg(sState);
	}

	return sRes;
}

bool MFieldObjCheckBox::canGetFocus()
{
	QWidget *p = getWidget();
	bool bRetVal = false;

	if(p!=NULL)
		if(p->isVisible() && !m_fieldData.isReadOnly())
			bRetVal = true;

	return bRetVal;
}


//-----------------------------------------------
bool MFieldObjRadioButton::connectToWidget(QWidget *parentForm, MInputExField* pData)
{
	m_fieldData = *pData;

	m_pRadio = parentForm->findChild<QRadioButton*>(m_fieldData.getName());

	if(m_pRadio!=NULL)
	{
		m_bObjConnected = true;
		updateDataToWidget();
		connect(m_pRadio,SIGNAL(toggled(bool)), this, SLOT(onDataChanged(bool)));
	}
	else
	{
		TRACE_W(QString( "MFieldObjRadioButton::connectToWidget: Unable to find QRadioButton:%1")
									.arg(m_fieldData.getName()));
	}


	return m_bObjConnected;
}

void MFieldObjRadioButton::updateDataToWidget()
{
	if(m_pRadio==NULL)
		return;
	m_bUpdateToWidgetInProgress = true;

	QString sText =	m_fieldData.getText();
	MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	QString sTranslation;
	pLang->getLabelTranslation(sText, MIhmLanguages::enuTranslTargetDesktop, sTranslation);
	m_pRadio->setText(sTranslation);
	
	QString sValue = m_fieldData.getValue();
	bool bState =((sValue=="0")||(sValue=="")||(sValue=="false"))?false:true;

	m_pRadio->setChecked(bState);
	m_pRadio->setEnabled(!m_fieldData.isReadOnly());

	m_bUpdateToWidgetInProgress = false;
}

void MFieldObjRadioButton::onDataChanged(bool bNewState)
{
	//do not execute if widget is updated by module
	if(m_bUpdateToWidgetInProgress) 
		return;

	QString sOldValue = m_fieldData.getValue();

	bool bOldState =((sOldValue=="0")||(sOldValue=="")||(sOldValue=="false"))?false:true;

	if(bNewState!=bOldState)
	{
		m_fieldData.setValue(getCurrentValue());
		emit dataChanged();
	}
}

QString MFieldObjRadioButton::getCurrentValue()
{
	QString sValue = (m_pRadio->isChecked())?"1":"0";

	return sValue;
}

QString MFieldObjRadioButton::getEncStrValue()
{
	QString sRes;

	if(m_pRadio!=NULL)
	{
		int iState;

		iState = (m_pRadio->isChecked())?1:0;
		sRes = QString("%1~%2").arg(m_fieldData.getName()).arg(iState);
	}

	return sRes;
}

bool MFieldObjRadioButton::canGetFocus()
{
	QWidget *p = getWidget();
	bool bRetVal = false;

	if(p!=NULL)
		if(p->isVisible() && !m_fieldData.isReadOnly())
			bRetVal = true;

	return bRetVal;
}
//-----------------------------------------------


bool MFieldObjComboBox::connectToWidget(QWidget *parentForm, MInputExField* pData)
{
	m_fieldData = *pData;

	m_pComboBoxFrame = parentForm->findChild<QFrame*>(m_fieldData.getName());

	if(m_pComboBoxFrame!=NULL)
	{
		m_pComboBox = new MIhmComboBox(m_pComboBoxFrame);
		QVBoxLayout *frLayout = new QVBoxLayout(m_pComboBoxFrame);
		frLayout->addWidget(m_pComboBox);
		frLayout->setSpacing(0);
		frLayout->setContentsMargins(0,0,0,0);

		m_bObjConnected = true;
		updateDataToWidget();
		connect(m_pComboBox,SIGNAL(currentIndexChanged(int)), this, SLOT(onValueChanged(int)));
	}
	else
	{
		TRACE_W(QString( "MFieldObjComboBox::connectToWidget: Unable to find QComboBox:%1")
									.arg(m_fieldData.getName()));
	}


	return m_bObjConnected;
}


void MFieldObjComboBox::updateDataToWidget()
{
	if(m_pComboBox==NULL)
		return;

	m_bUpdateToWidgetInProgress = true;
	
	m_pComboBox->clear();

	QList <MOptionData*> *pList = m_fieldData.getOptions();

	QString sText, sValue, sTranslation ;
	MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();

	for (int i=0;i<pList->size();i++)
	{
		sText = pList->at(i)->m_sText;
		pLang->getLabelTranslation(sText, MIhmLanguages::enuTranslTargetDesktop, sTranslation);
		sValue = pList->at(i)->m_sValue;

		m_pComboBox->addItem(sTranslation, sValue); 
	}

	int iIndex = m_pComboBox->findData(m_fieldData.getValue());

	if(iIndex>=0)
		m_pComboBox->setCurrentIndex(iIndex);
	
	m_pComboBox->setEnabled(!m_fieldData.isReadOnly());

	m_bUpdateToWidgetInProgress = false;
}

void MFieldObjComboBox::onValueChanged(int iIndex)
{
	//do not execute if widget is updated by module
	if(m_bUpdateToWidgetInProgress) 
		return;

	QString sOldValue = m_fieldData.getValue();

	QString sNewValue = getCurrentValue();

	if(sNewValue!=sOldValue)
	{
		m_fieldData.setValue(getCurrentValue());
		emit dataChanged();
	}
}

QString MFieldObjComboBox::getCurrentValue()
{
	QString sValue;
	
	if(m_pComboBox!=NULL)
	{
		QVariant var = m_pComboBox->itemData(m_pComboBox->currentIndex());
		sValue = var.toString();
	}

	return sValue;
}


QString MFieldObjComboBox::getEncStrValue()
{
	QString sRes;

	if(m_pComboBox!=NULL)
	{

		sRes = QString("%1~%2").arg(m_fieldData.getName()).arg(getCurrentValue());
	}

	return sRes;
}

bool MFieldObjComboBox::isValidValue()
{
	if(getCurrentValue()!="")
	{
		return true;
	}

	return false;
}

bool MFieldObjComboBox::canGetFocus()
{
	QWidget *p = getWidget();
	bool bRetVal = false;

	if(p!=NULL)
		if(p->isVisible() && !m_fieldData.isReadOnly())
			bRetVal = true;

	return bRetVal;
}
//-----------------------------------------------

void MFieldObjListWidget::setFilter(QString sFilter, enuFilterType e)
{
	if(sFilter!=m_sFilter)
	{
		m_eFilterType = e;
		m_sFilter = sFilter;
		updateDataToWidget();
	}
}


bool MFieldObjListWidget::connectToWidget(QWidget *parentForm, MInputExField* pData)
{
	m_fieldData = *pData;

	QString sName = m_fieldData.getName();
	m_pListWidgetFrame = parentForm->findChild<QFrame*>(sName);

	if(m_pListWidgetFrame!=NULL)
	{
		m_pListWidget = new MIhmListWidget(m_pListWidgetFrame);
		QVBoxLayout *frLayout = new QVBoxLayout(m_pListWidgetFrame);
		frLayout->addWidget(m_pListWidget);
		frLayout->setSpacing(0);
		frLayout->setContentsMargins(0,0,0,0);

		m_bObjConnected = true;
		updateDataToWidget();
		connect(m_pListWidget,SIGNAL(currentRowChanged(int)), this, SLOT(onValueChanged(int)));
	}
	else
	{
		TRACE_W(QString( "MFieldObjListWidget::connectToWidget: Unable to find QListWidget:%1")
									.arg(sName));
	}


	return m_bObjConnected;
}

void MFieldObjListWidget::updateDataToWidget()
{
	if(m_pListWidget==NULL)
		return;

	m_bUpdateToWidgetInProgress = true;

	QListWidgetItem * p;

	while(m_pListWidget->count()>0)
	{
		p = m_pListWidget->takeItem(0);
		delete p;
	}

	QList <MOptionData*> *pList = m_fieldData.getOptions();


	QString sText, sValue, sTranslation;

	QListWidgetItem * pNew;
	bool currentItemFound = false;
	MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();

	for (int i=0;i<pList->size();i++)
	{
		sText = pList->at(i)->m_sText;
		pLang->getLabelTranslation(sText, MIhmLanguages::enuTranslTargetDesktop, sTranslation);
		sValue = pList->at(i)->m_sValue;

		if(m_sFilter!="")
		{
			int iPos = sText.indexOf(m_sFilter, 0, Qt::CaseInsensitive);

			if (m_eFilterType == enuFILTER_BEGINS_WITH)
			{
				if (iPos != 0)
					continue;

			}
			else
			{
				if (iPos == -1)
					continue;
			}
		}	

		pNew = new QListWidgetItem(sTranslation);
		pNew->setData(Qt::UserRole, sValue);

		m_pListWidget->addItem(pNew);

		if(sValue == m_fieldData.getValue() && sValue!="")
		{
			m_pListWidget->setCurrentItem(pNew);
			pNew->setSelected(true);
			currentItemFound = true;
		}

		if(m_pListWidget->count()>IHM_SAISIE_EX_MAX_ROWS_IN_OPTION_LIST)
			break;
	}
	
	if(!currentItemFound)
	{
		m_pListWidget->setCurrentRow(0);
		pNew = m_pListWidget->currentItem();

		if(pNew!=NULL)
			pNew->setSelected(true);
	}



	m_pListWidget->setEnabled(!m_fieldData.isReadOnly());

	m_bUpdateToWidgetInProgress = false;

}

void MFieldObjListWidget::onValueChanged(int iIndex)
{
	//do not execute if widget is updated by module
	if(m_bUpdateToWidgetInProgress) 
		return;

	QString sOldValue = m_fieldData.getValue();

	QString sNewValue = getCurrentValue();

	if(sNewValue!=sOldValue)
	{
		m_fieldData.setValue(getCurrentValue());
		emit dataChanged();
	}
}

QString MFieldObjListWidget::getCurrentValue()
{
	QString sValue;
	
	if(m_pListWidget!=NULL)
	{
		QListWidgetItem * p = m_pListWidget->currentItem();
		if(p!=NULL)
		{
			QVariant var = p->data(Qt::UserRole);
			sValue = var.toString();
		}
	}

	return sValue;
}


QString MFieldObjListWidget::getEncStrValue()
{
	QString sRes;

	sRes = QString("%1~%2").arg(m_fieldData.getName()).arg(getCurrentValue());

	return sRes;
}

bool MFieldObjListWidget::isValidValue()
{
	
	if (getCurrentValue() != "" || 
		m_fieldData.getSecondaryValidator()=="*") //To be enable Accept button when nothing is selected in list (filtered data not is not found e.g."R1 data") - validator="*" should be defined in the ListWidget fieled definition
	{
		return true;
	}

	return false;
}

bool MFieldObjListWidget::canGetFocus()
{
	QWidget *p = getWidget();
	bool bRetVal = false;

	if(p!=NULL)
		if(p->isVisible() && !m_fieldData.isReadOnly())
			bRetVal = true;

	return bRetVal;
}

//-----------------------------------------------


bool MFieldObjLineEdit::connectToWidget(QWidget *parentForm, MInputExField* pData)
{
	m_fieldData = *pData;

	m_pLineEdit = parentForm->findChild<QLineEdit*>(m_fieldData.getName());

	if(m_pLineEdit!=NULL)
	{
		m_bObjConnected = true;
		updateDataToWidget();
		connect(m_pLineEdit,SIGNAL(textChanged(const QString &)), this, SLOT(onValueChanged(const QString &)));
	}
	else
	{
		TRACE_W(QString( "MFieldObjLineEdit::connectToWidget:Unable to find QLineEdit:%1")
									.arg(m_fieldData.getName()));
	}


	return m_bObjConnected;
}

void MFieldObjLineEdit::updateDataToWidget()
{
	if(m_pLineEdit==NULL)
		return;

	m_bUpdateToWidgetInProgress = true;

	m_pLineEdit->setText(m_fieldData.getValue());


	if(m_fieldData.getType()== MInputExField::enuDLG_EX_FIELD_HIDDEN_EDIT)
	{
		m_pLineEdit->setVisible(false);
		m_pLineEdit->setEnabled(false);
	}
	else // if(m_fieldData.getType()== MInputExField::enuDLG_EX_FIELD_LINE_EDIT)
		m_pLineEdit->setEnabled(!m_fieldData.isReadOnly());

	m_bUpdateToWidgetInProgress = false;
}


void MFieldObjLineEdit::onValueChanged(const QString &sNewValue)
{
	//do not execute if widget is updated by module
	if(m_bUpdateToWidgetInProgress) 
		return;

	QString sOldValue = m_fieldData.getValue();

	if(sNewValue!=sOldValue)
	{
		m_fieldData.setValue(sNewValue);
		emit dataChanged();
	}
}

QString MFieldObjLineEdit::getCurrentValue()
{
	QString sValue;
	
	if(m_pLineEdit!=NULL)
	{
		sValue = m_pLineEdit->text();
	}

	return sValue;
}

QString MFieldObjLineEdit::getEncStrValue()
{
	QString sRes;

	if(m_pLineEdit!=NULL)
	{
		QString sValue =m_pLineEdit->text();

		sRes = QString("%1~%2").arg(m_fieldData.getName()).arg(getCurrentValue());
	}

	return sRes;
}

bool MFieldObjLineEdit::isValidValue()
{
	QString sValue =m_pLineEdit->text();
	QString sSecondaryValidator = m_fieldData.getSecondaryValidator();
	
	if(sSecondaryValidator == "") 
		return true; //if no secondary validator defined
	else
	{
		//verify with secondary validator
		QRegularExpression rx(sSecondaryValidator);
		QRegularExpressionValidator secValidator(rx, this);
		
		int iPos = 0;
		
		if(secValidator.validate(sValue, iPos)== QValidator::Acceptable)
			return true;
		else
		{
			return false;
			TRACE_D(QString("MFieldObjLineEdit::isValidValue: Secondary regex validator [%1] failed for value [%2] in input %3!")
									.arg(sSecondaryValidator)
									.arg(sValue)
									.arg(m_fieldData.getName()));
		
		}
	}

	return false;
}

bool MFieldObjLineEdit::canGetFocus()
{
	if(m_pLineEdit!=NULL)
	{
		return !m_fieldData.isReadOnly() && 
			m_pLineEdit->isVisible() && 
			m_pLineEdit->isEnabled();
	}


	return false;
}


//-----------------------------------------------


//-----------------------------------------------
MFieldObjLineEditEx::MFieldObjLineEditEx()
{
	m_pMLineEdit = NULL;
}

MFieldObjLineEditEx::~MFieldObjLineEditEx()
{
	if(m_pMLineEdit!=NULL) 
			delete m_pMLineEdit;

}

bool MFieldObjLineEditEx::connectToWidget(QWidget *parentForm, MInputExField* pData)
{
	m_fieldData = *pData;

	m_pContainerFrame = parentForm->findChild<QFrame*>(m_fieldData.getName());

	if(m_pContainerFrame!=NULL)
	{
		if(m_pMLineEdit!=NULL)
		{
			delete m_pMLineEdit; m_pMLineEdit = NULL;
		}

		m_pMLineEdit = new MLineEdit(m_pContainerFrame);
		QHBoxLayout * pNewLayout = new QHBoxLayout(m_pContainerFrame);
		pNewLayout->addWidget(m_pMLineEdit);
		pNewLayout->setSpacing(0);
		pNewLayout->setContentsMargins(0,0,0,0);
		
		if(!initLineEditFormat())
		{
			TRACE_W(QString( "MFieldObjLineEdit::connectToWidget: Invalid format for LineEditEx:%1")
									.arg(m_fieldData.getName()));
		}
		
		m_bObjConnected = true;
		updateDataToWidget();
		connect(m_pMLineEdit,SIGNAL(textChanged(const QString &)), this, SLOT(onValueChanged(const QString &)));
	}
	else
	{
		TRACE_W(QString( "MFieldObjLineEditEx::connectToWidget: Unable to find QFrame:%1: Note that the LineEditEx require an QFrame object in UI file to be correctly created!")
									.arg(m_fieldData.getName()));
	}

	return m_bObjConnected;
}




bool MFieldObjLineEditEx::initLineEditFormat()
{
	QString	sFormatDetail = m_fieldData.getFormatDetail();

	if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_TEXT)
	{
		if(m_fieldData.getMinLen()>-1 && m_fieldData.getMaxLen()>-1)
		{
			QString sReg;

			QString sCharRegex = m_fieldData.getFormatStr().mid(1);

			if (sCharRegex.size() > 0)
			{
				sReg = sCharRegex;
			}
			else
			{
				//matches any character if minimal m_iMinLen and maximal m_iMaxLen
				sReg = QString(".");
			}

			sReg = sReg + QString("{%1,%2}").arg(m_fieldData.getMinLen()).arg(m_fieldData.getMaxLen());

			QRegularExpression rx(sReg);

			if (rx.isValid())
			{
				TRACE_D(QString("MFieldObjLineEditEx::initLineEditFormat: Regex generated for the format [%1] in field [%2] to verify string input! Regex[%3]")
					.arg(m_fieldData.getFormatStr())
					.arg(m_fieldData.getName())
					.arg(sReg));

				QRegularExpressionValidator *validator = new QRegularExpressionValidator(rx, this);
				m_pMLineEdit->setValidator(validator);
			}
			else
			{
				TRACE_W(QString("MFieldObjLineEditEx::initLineEditFormat: Invalid regex generated for the format [%1] in field [%2] to verify string input! Regex[%3]")
					.arg(m_fieldData.getFormatStr())
					.arg(m_fieldData.getName())
					.arg(sReg));
			}

			m_pMLineEdit->setMaxLength(m_fieldData.getMaxLen());
			return true;
		}
		else
		{
			TRACE_W(QString("MFieldObjLineEditEx::initLineEditFormat: Both min_len and max_len must be defined for the format [%1] in field [%2] to verify string input size!")
									.arg(m_fieldData.getFormatStr())
									.arg(m_fieldData.getName()));

		}

	}	
	else if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_MASK)
	{
		if(sFormatDetail!="")
		{
			m_pMLineEdit->setTxtInputMask(sFormatDetail);
		}

		return true;
	}	
	else if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_RIGHTMASK)
	{
		if(sFormatDetail!="")
		{
			if(verifyRightMask(sFormatDetail))
			{
				m_pMLineEdit->setRightMask(m_fieldData.getInputMask(), m_fieldData.getMaskPlaceholder());
				return true;
			}
			else
			{
				TRACE_W(QString("MFieldObjLineEditEx::initLineEditFormat: Invalid mask for rightmask [%1] for field [%2] Mask should be i.e. [9999;0]!")
									.arg(sFormatDetail)
									.arg(m_fieldData.getName()));

			}
		}
	}	
	else if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_NUMBER)
	{
		if(m_pMLineEdit->setNumberFormat(sFormatDetail))
					return true;

	}	
	else if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_CALCULATION)
	{
		m_pMLineEdit->setEnabled(false);

		return true;
	}	
	else if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY || 
			m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY_MOD16||
			m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY_MOD16_M3)
	{
		QString sReg, sRep;
		//matches any digit character if minimal m_iMinLen and maximal m_iMaxLen
		sReg = QString("[\\dA-Fa-f]");

		if(m_fieldData.getMaxLen()>0)
		{
			m_pMLineEdit->setMaxLength(m_fieldData.getMaxLen());	
			sRep = QString("{%1,%2}").arg(m_fieldData.getMinLen()).arg(m_fieldData.getMaxLen());
		}
		else
		{
			sRep = "*";
		}

		sReg.append(sRep);
			
		QRegularExpression rx(sReg);
		QRegularExpressionValidator*validator = new QRegularExpressionValidator(rx, this);
		m_pMLineEdit->setValidator(validator);

		return true;	
	}	
	else 
	{
		//default:
		TRACE_W(QString("MFieldObjLineEditEx::initLineEditFormat: Unknown format %1 !")
									.arg(m_fieldData.getFormatType()));
	}

	return false;
}


//verify if right mask is correctly defined 
// it should be all digits 
bool MFieldObjLineEditEx::verifyRightMask(QString sRightMask)
{
	int iLen = sRightMask.size();

	if(iLen < 2)
		return true;

	QString sMaskToCheck;
	sMaskToCheck = sRightMask;
	
	if(iLen>2)
	{
		if(sRightMask.at(iLen-2) == ';')
		{
			sMaskToCheck = sRightMask.left(iLen-2);
		}
	}


	for(int i=0;i<sMaskToCheck.size();i++)
	{
		if(sMaskToCheck.at(i)!='9' &&
			sMaskToCheck.at(i)!='.' && 
			sMaskToCheck.at(i)!=',')
			return false;
	}

	return true;
}



void MFieldObjLineEditEx::updateDataToWidget()
{
	if(m_pMLineEdit==NULL)
		return;

	m_bUpdateToWidgetInProgress = true;

	QString sValue = m_fieldData.getValue();

	bool bValid = true;
	const QValidator * pVal = m_pMLineEdit->validator();

	if(pVal!=NULL)
	{
		int iPos;

		if(pVal->validate(sValue, iPos)== QValidator::Acceptable)
			bValid = true;
		else
			bValid = false;
	}
	
	if(bValid)
		m_pMLineEdit->setText(sValue);
	else
	{
		TRACE_W(QString("MFieldObjLineEditEx::updateDataToWidget: Invalid value[%1] for input %2. Check defined format!")
									.arg(sValue)
									.arg(m_fieldData.getName()));
	}


	if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_CALCULATION)
		m_pMLineEdit->setEnabled(false);
	else
		m_pMLineEdit->setEnabled(!m_fieldData.isReadOnly());

	m_bUpdateToWidgetInProgress = false;
}


void MFieldObjLineEditEx::onValueChanged(const QString &sNewValue)
{
	//do not execute if widget is updated by module
	if(m_bUpdateToWidgetInProgress) 
		return;

	QString sOldValue = m_fieldData.getValue();

	if(sNewValue!=sOldValue)
	{
		m_fieldData.setValue(sNewValue);
		emit dataChanged();
	}


	//Input fields of type Masked input will trigger move focus to next input field by default
	if (m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_MASK) 
	{
		int iPos = m_pMLineEdit->cursorPosition();
		QString sMask = m_pMLineEdit->inputMask();

		if (sMask != "" && iPos == sMask.size() - 2 )
		{
			if (sNewValue.right(1) != sMask.right(1))
			{
				emit focusNext();
			}
		}
	}
}

QString MFieldObjLineEditEx::getCurrentValue()
{
	QString sValue;
	
	if(m_pMLineEdit!=NULL)
	{
		sValue = m_pMLineEdit->displayText();
	}

	return sValue;
}

QString MFieldObjLineEditEx::getEncStrValue()
{
	QString sRes;

	if(m_pMLineEdit!=NULL)
	{
		QString sValue =m_pMLineEdit->text();

		sRes = QString("%1~%2").arg(m_fieldData.getName()).arg(getCurrentValue());
	}

	return sRes;
}

bool MFieldObjLineEditEx::isValidValue()
{
	bool bRetVal = false;

	QString sValue = getCurrentValue();
	
	
	if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_CALCULATION)
	{
		bRetVal = true;
	}
	else if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY ||
			m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY_MOD16 ||
			m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY_MOD16_M3 )
	{
		if(m_fieldData.getMinLen()>-1 && sValue.size()<m_fieldData.getMinLen())
			bRetVal = false;
		else if(m_fieldData.getMaxLen()>-1 && sValue.size()>m_fieldData.getMaxLen())
			bRetVal = false;
		else
		{
			if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY)
				bRetVal = MHelpFuncs::checkLuhnKey(sValue);
			else if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY_MOD16)
				bRetVal = MHelpFuncs::checkLuhnKeyMod16(sValue);
			else //if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY_MOD16_M3)
				bRetVal = MHelpFuncs::checkLuhnKeyMod16_M3(sValue);
				
		}
	}
	if (m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_MASK)
	{
		bRetVal = m_pMLineEdit->hasAcceptableInput();
	}
	else
	{
		const QValidator * pVal = m_pMLineEdit->getValidator();

		if(pVal!=NULL)
		{
			int iPos = 0;
			
			//if nothing set it is considered to be valid for primary validator
			//secondary validator is to be set to assure that something is entered
			if(sValue=="") 
			{
				bRetVal = true;	
			}
			else if(pVal->validate(sValue, iPos)== QValidator::Acceptable)
				bRetVal = true;
			else
			{
				TRACE_D(QString("MFieldObjLineEditEx::isValidValue: Format validator failed for value [%1] in input %2!")
									.arg(sValue)
									.arg(m_fieldData.getName()));

				bRetVal = false;
			}
		}
		else //if no validator it means that it is true
			bRetVal = true;
		

	}

	//if true by now additionally verify with secondary RegEx validator 
	//	(It is to be set as validator argument in input definition file  )
	if(bRetVal) 
	{	
		QString sSecondaryValidator = m_fieldData.getSecondaryValidator();
		
		if(sSecondaryValidator == "") 
			return true; //if no secondary validator defined
		else
		{
			//verify with secondary validator
			QRegularExpression rx(sSecondaryValidator);
			QRegularExpressionValidator secValidator(rx, this);
			
			int iPos = 0;
			
			if(secValidator.validate(sValue, iPos)== QValidator::Acceptable)
				bRetVal = true;
			else
			{
				bRetVal = false;
				TRACE_D(QString("MFieldObjLineEditEx::isValidValue: Secondary regex validator [%1] failed for value [%2] in input %3!")
									.arg(sSecondaryValidator)
									.arg(sValue)
									.arg(m_fieldData.getName()));
			
			}
			

			return bRetVal;
		}

	}
	else
		return false;
}

void MFieldObjLineEditEx::updateValue(QString sNewValue)
{
	if(m_fieldData.getFormatType() == MInputExField::enuFIELD_FORMAT_CALCULATION)
	{
		m_fieldData.setValue(sNewValue);
		m_pMLineEdit->setText(sNewValue);
	}
}

bool MFieldObjLineEditEx::canGetFocus()
{
	bool bRetVal = false;

	if(m_pMLineEdit!=NULL)
		if(m_pMLineEdit->isVisible() && m_pMLineEdit->isEnabled())
			bRetVal = true;

	return bRetVal;
}


//-----------------------------------------------






bool MFieldObjTextEdit::connectToWidget(QWidget *parentForm, MInputExField* pData)
{
	m_fieldData = *pData;

	m_pTextEdit = parentForm->findChild<QTextEdit*>(m_fieldData.getName());

	if(m_pTextEdit!=NULL)
	{
		m_bObjConnected = true;
		updateDataToWidget();
		connect(m_pTextEdit,SIGNAL(textChanged()), this, SLOT(onValueChanged()));
	}
	else
	{
		TRACE_W(QString( "MFieldObjTextEdit::connectToWidget: Unable to find QTextEdit:%1")
									.arg(m_fieldData.getName()));
	}


	return m_bObjConnected;
}

void MFieldObjTextEdit::onValueChanged()
{
	//do not execute if widget is updated by module
	if(m_bUpdateToWidgetInProgress) 
		return;

	QString sOldValue = m_fieldData.getValue();
	QString sNewValue = m_pTextEdit->toPlainText();

	if(sNewValue!=sOldValue)
	{
		m_fieldData.setValue(sNewValue);
		emit dataChanged();
	}
}

QString MFieldObjTextEdit::getCurrentValue()
{
	QString sValue;
	
	if(m_pTextEdit!=NULL)
	{
		sValue = m_pTextEdit->toPlainText();
	}

	return sValue;
}

bool MFieldObjTextEdit::isValidValue()
{
	bool bRetVal = false;

	QString sSecondaryValidator = m_fieldData.getSecondaryValidator();
	
	if(sSecondaryValidator == "") 
		return true; //if no secondary validator defined
	else
	{
		QString sValue = getCurrentValue();
		//verify with secondary validator
		QRegularExpression rx(sSecondaryValidator);
		QRegularExpressionValidator secValidator(rx, this);
		
		int iPos = 0;
		
		if(secValidator.validate(sValue, iPos)== QValidator::Acceptable)
			bRetVal = true;
		else
		{
			bRetVal = false;
		}
		
		
		return bRetVal;
	}

}


void MFieldObjTextEdit::updateDataToWidget()
{
	if(m_pTextEdit==NULL)
		return;
	m_bUpdateToWidgetInProgress = true;

	QString sText = m_fieldData.getValue();
	
	if(!m_fieldData.isValueHtml())
	{
		sText = MHelpFuncs::replaceNewLinesForDsk(sText);
		m_pTextEdit->setPlainText(sText);
	}
	else
	{
		m_pTextEdit->setHtml(sText);
	}

	m_pTextEdit->setReadOnly(m_fieldData.isReadOnly());

	m_bUpdateToWidgetInProgress = false;
}

QString MFieldObjTextEdit::getEncStrValue()
{
	QString sRes;

	if(m_pTextEdit!=NULL)
	{
		sRes = QString("%1~%2").arg(m_fieldData.getName()).arg(getCurrentValue());
	}

	return sRes;
}

bool MFieldObjTextEdit::canGetFocus()
{
	QWidget *p = getWidget();
	bool bRetVal = false;

	if(p!=NULL)
		if(p->isVisible())
			bRetVal = true;

	return bRetVal;
}

//-----------------------------------------------


bool MFieldObjLabel::connectToWidget(QWidget *parentForm, MInputExField* pData)
{
	m_fieldData = *pData;

	m_pLabel = parentForm->findChild<QLabel*>(m_fieldData.getName());

	if(m_pLabel!=NULL)
	{
		m_bObjConnected = true;
		updateDataToWidget();
	}
	else
	{
		TRACE_W(QString( "MFieldObjLabel::connectToWidget: Unable to find QLabel:%1")
									.arg(m_fieldData.getName()));
	}

	return m_bObjConnected;
}

void MFieldObjLabel::updateDataToWidget()
{
	if(m_pLabel==NULL)
		return;

	QString sText = m_fieldData.getText();

	if(sText!="")
	{
		MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
		pLang->getLabelTranslation(sText, MIhmLanguages::enuTranslTargetDesktop, sText);
	}

	m_pLabel->setText(sText);

}


QString MFieldObjLabel::getEncStrValue()
{

	return QString("");
}


//-----------------------------------------------
//--Class  MFieldObjLuhnCheckLabel 
//         - used for MFieldObjLineEditEx LUHN_KEY format type
//          to show correct luhn key check icon
//-----------------------------------------------

bool MFieldObjLuhnCheckLabel::connectToWidget(QWidget *parentForm, MInputExField* pData)
{
	m_fieldData = *pData;

	m_pLabel = parentForm->findChild<QLabel*>(m_fieldData.getName());

	if(m_pLabel!=NULL)
	{
		m_bObjConnected = true;
		updateDataToWidget();
	}
	else
	{
		TRACE_W(QString( "MFieldObjLuhnCheckLabel::connectToWidget: Unable to find QLabel:%1")
									.arg(m_fieldData.getName()));
	}

	return m_bObjConnected;
}

void MFieldObjLuhnCheckLabel::initLuhnKeyIcons(QString sImgLuhnOK, QString sImgLuhnNOK)
{
	if(!m_PixmapLuhnOK.load(sImgLuhnOK))
	{
		TRACE_W(QString( "MFieldObjLuhnCheckLabel::initLuhnKeyIcons: Unable to load sImgLuhnOK:%1")
									.arg(sImgLuhnOK));
	}

	if(!m_PixmapLuhnNOK.load(sImgLuhnNOK))
	{
		TRACE_W(QString( "MFieldObjLuhnCheckLabel::initLuhnKeyIcons: Unable to load sImgLuhnNOK:%1")
									.arg(sImgLuhnNOK));
	}



}

void MFieldObjLuhnCheckLabel::updateLuhnKeyCheckIcon(bool bOK)
{
	if(m_pLabel!=NULL)
	{
		if(bOK)
		{
			if(!m_PixmapLuhnOK.isNull())
				m_pLabel->setPixmap(m_PixmapLuhnOK);
		}
		else
			if(!m_PixmapLuhnNOK.isNull())
				m_pLabel->setPixmap(m_PixmapLuhnNOK);

	}

}

void MFieldObjLuhnCheckLabel::updateDataToWidget()
{
	if(m_pLabel!=NULL)
		m_pLabel->setText("");
}

QString MFieldObjLuhnCheckLabel::getEncStrValue()
{
	return QString("");
}



//-----------------------------------------------
bool MFieldObjButton::connectToWidget(QWidget *parentForm, MInputExField* pData)
{
	m_fieldData = *pData;

	m_pButton = parentForm->findChild<QPushButton*>(m_fieldData.getName());

	if(m_pButton!=NULL)
	{
		m_bObjConnected = true;
		updateDataToWidget();
	}
	else
	{
		TRACE_W(QString( "MFieldObjButton::connectToWidget: Unable to find QPushButton:%1")
									.arg(m_fieldData.getName()));
	}

	return m_bObjConnected;
}


void MFieldObjButton::updateDataToWidget()
{
	if(m_pButton==NULL)
		return;

	QString sText = m_fieldData.getText();

	MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	QString sTranslation;
	pLang->getLabelTranslation(sText, MIhmLanguages::enuTranslTargetDesktop, sTranslation);

	m_pButton->setText(sTranslation);

	m_pButton->setEnabled(!m_fieldData.isReadOnly());
	m_pButton->setFocusPolicy(Qt::StrongFocus);

}


QString MFieldObjButton::getEncStrValue()
{
	return QString("");
}

bool MFieldObjButton::canGetFocus()
{
	return !m_fieldData.isReadOnly();
}

MFieldObjButton * MFieldObjButton::findFieldObjectByKeyID(QList <MInputExFieldObj*> *plst, QString sKeyID)
{
	MFieldObjButton *pRetObj = NULL;
	
	for (int i = 0; i < plst->size(); ++i) 
	{
		if(plst->at(i)->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON)
		{
			MFieldObjButton *pCurrent = (MFieldObjButton*)plst->at(i);
			if(pCurrent->getKeyID() == sKeyID)
			{
				pRetObj = pCurrent;
				break;
			}

		}
	}
	
	return pRetObj;
}

MFieldObjButton * MFieldObjButton::findNextFieldObjectByKeyID(MFieldObjButton *pCurrent, QList <MInputExFieldObj*> *plst, QString sKeyID)
{
	MFieldObjButton *pRetObj = NULL;
	bool bFound = false;

	for (int i = 0; i < plst->size(); ++i) 
	{
		if(!bFound)
		{
			if(plst->at(i) == pCurrent)
				bFound = true;

			continue;
		}

		if(plst->at(i)->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON)
		{
			MFieldObjButton *pCurrent = (MFieldObjButton*)plst->at(i);
			if(pCurrent->getKeyID() == sKeyID)
			{
				pRetObj = pCurrent;
				break;
			}

		}
	}
	
	return pRetObj;
}
