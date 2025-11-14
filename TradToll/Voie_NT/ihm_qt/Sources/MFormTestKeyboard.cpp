
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QFrame>
#include <QKeyEvent>
#include <QComboBox>
#include <QAction>
#include <QModelIndex>

#include <QtUiTools>

#include "MFormTestKeyboard.h"
#include "MIhmConfigGeneral.h"
#include "MHelpFuncs.h"
#include "MInputExFieldObj.h"
#include "MIhmComboBox.h"
#include "MIhmListWidget.h"
#include "MTracer.h"


#define _LIST_VIEW_ACTIONS				"listViewActions"
#define IHM_LABEL_KBD_KEY								"KBD_KEY"
#define IHM_LABEL_KBD_SPECIAL_KEY						"KBD_SPECIAL_KEY"
#define IHM_LABEL_INTERIOR_BARCODE						"BARCODE_INTERIOR"
#define IHM_LABEL_VL_BARCODE							"BARCODE_VL"
#define IHM_LABEL_PL_BARCODE							"BARCODE_PL"
#define IHM_LABEL_ZIP_READER							"ZIP_READER"
#define IHM_LABEL_DESC_BTN_ID							"LABEL_DESC_BTN_ID_"
#define IHM_LABEL_KBD_TEST_TITLE						"LABEL_KBD_TEST_TITLE"



MFormTestKeyboard::MFormTestKeyboard(QWidget *parent)
    : MFormInputEx(parent)
{
	m_pListWidget = NULL;
	setWindowFlags(Qt::Dialog
						| Qt::MSWindowsFixedSizeDialogHint| Qt::WindowSystemMenuHint);

	m_bTemplateLoaded = false;
	m_sResult = "";
}

MFormTestKeyboard::~MFormTestKeyboard()
{
	if(m_pListWidget!=NULL)
		m_pListWidget->clear();
}


bool MFormTestKeyboard::loadTemplate(QString sUIFilePath)
{
	bool bRetVal = false;
	QUiLoader loader;

	TRACE_D(QString("MFormTestKeyboard::loadTemplate: sUIFilePath:%1").arg(sUIFilePath));
	
	QFile file(sUIFilePath);
	if(file.open(QFile::ReadOnly))
	{
		m_dynFormWidget = loader.load(&file, this);
		file.close();
		m_sTemplateSize = m_dynFormWidget->size();

		m_frContainer = this->findChild<QFrame*>("frContainer");

		QPalette pal1 = m_dynFormWidget->palette();
		this->setAttribute(Qt::WA_WindowPropagation, false);
		this->setPalette(pal1);

		if(m_frContainer != NULL)
		{
			
			m_layout = new QVBoxLayout(this);
			this->setContentsMargins(0,0,0,0);
			m_layout->addWidget(m_frContainer);
			m_layout->setSpacing(0);
			m_layout->setContentsMargins(0,0,0,0);

			setWindowTitle(getKeyboardTestTitleLabel());

			bRetVal = true;
		}

		m_bTemplateLoaded = true;
	}
	
	if(!bRetVal)
	{
		TRACE_W(QString( "MFormTestKeyboard::loadTemplate: Error loading %1").arg(sUIFilePath.toLatin1().data()));
	}

	return bRetVal;
}


bool MFormTestKeyboard::connectDefinedObjects()
{

	while (!m_lstFieldObjList.isEmpty())
		delete m_lstFieldObjList.takeFirst();
	
	QList <MInputExField *> * plstFieldList = m_oReq.getInputFields();

	MInputExField * pCurrentField;

	for (int i=0; i<plstFieldList->size();i++)
	{
		pCurrentField = plstFieldList->at(i);
		
		MInputExFieldObj * pNewObj = MInputExFieldObj::createNewFieldObj(pCurrentField);

		if(pNewObj!=NULL)
		{
			if(pNewObj->connectToWidget(this, pCurrentField))
			{
				if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON_CANCEL)
				{
					m_bCancelButton = true;
					QWidget * w = pNewObj->getWidget();

					w->setFocusPolicy(Qt::NoFocus);
					
					if(w!=NULL)
						QObject::connect(w, SIGNAL(clicked()), this, SLOT(onButtonCancel()));
				}
				else if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON)
				{
					QWidget * w = pNewObj->getWidget();
					w->setFocusPolicy(Qt::NoFocus);
				}
				else if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_LIST_WIDGET)
				{
					m_pListWidget = (MIhmListWidget *)pNewObj->getWidget();
					m_pListWidget->setFocusPolicy(Qt::NoFocus);
				}
				
				m_lstFieldObjList.append(pNewObj);
				
				//install the main event filter
 				//pNewObj->getWidget()->installEventFilter(this); //- do not use the usual Enter, Esc, Space key processing
				pNewObj->getWidget()->installEventFilter(m_pParentWindow);
			}
			else
			{
				delete pNewObj;
				TRACE_W(QString("MFormInputGen::connectDefinedObjects: connectToWidget for object %1 returned false!")
									.arg(pCurrentField->getName()));

				return false;
			}

		}	
		else
		{
			TRACE_W(QString("MFormInputGen::connectDefinedObjects: Unknown object type for object %1 !")
									.arg(pCurrentField->getName()));

			return false;
		}
	}
		
	return true;
}



void MFormTestKeyboard::onKeyDetected(int iKeyID, int iAsciiCode)
{
	QString sBtnID, sBtnTypeLbl, sBtnText, sBtnDescription;
	QString sAsciiBtnLabel, sBtnLbl;
	QWidget * pCurrentWidget = NULL;
	bool bValidKeyPressed = false;

	// Check on which ID we will execute object search
	if (iKeyID == 0) 
		sBtnID = QString::number(iAsciiCode);
	else
		sBtnID = QString::number(iKeyID);

	// We find object that is just pressed and we need to display it as pressed.
	MFieldObjButton * pDetectedObj = MFieldObjButton::findFieldObjectByKeyID(&m_lstFieldObjList, sBtnID);

	while(pDetectedObj != NULL) 
	{
		if(!pDetectedObj->isReadOnly())
		{
			pCurrentWidget = pDetectedObj->getWidget();
			
			((QAbstractButton*)pCurrentWidget)->animateClick();

			sBtnText = pDetectedObj->getKeyText();
			
			bValidKeyPressed = true;
		}

		pDetectedObj = MFieldObjButton::findNextFieldObjectByKeyID(pDetectedObj, &m_lstFieldObjList, sBtnID);
	}

	if(bValidKeyPressed)
	{
		sBtnText = getKeyTextTranslation(sBtnText);	
		sAsciiBtnLabel = QString("%1%2").arg(IHM_LABEL_DESC_BTN_ID).arg(sBtnID);
		sBtnDescription = getKeyTextTranslation(sAsciiBtnLabel);

		if(sBtnDescription == sAsciiBtnLabel)
			sBtnDescription = sBtnText;

		// Here we need to update "ActionList" with data of pressed button.
		sBtnLbl = QString("[%1] : %2 - %3").arg(sBtnText).arg(getKbdKeyTypeLabel()).arg(sBtnDescription);

		updateMsgList(sBtnLbl);
	}


}

void MFormTestKeyboard::onTollKeyDetected(int iKeyID, QString sDetectedString)
{
	QString sBtnTypeLbl, sBtnText, sBtnLabel, sBtnDescription;
	QString sBtnLbl;
	QWidget * pCurrentWidget = NULL;
	bool bValidKeyPressed = false;
	
	
	// We find object that is just pressed and we need to display it as pressed.
	MFieldObjButton * pDetectedObj = MFieldObjButton::findFieldObjectByKeyID(&m_lstFieldObjList, QString::number(iKeyID));
	
	while(pDetectedObj != NULL) 
	{
		if(!pDetectedObj->isReadOnly())
		{
			pCurrentWidget = pDetectedObj->getWidget();
			((QAbstractButton*)pCurrentWidget)->animateClick();
			
			sBtnText = pDetectedObj->getKeyText();
			bValidKeyPressed = true;
		}

		pDetectedObj = MFieldObjButton::findNextFieldObjectByKeyID(pDetectedObj, &m_lstFieldObjList, QString::number(iKeyID));
	}

	if(bValidKeyPressed)
	{
		sBtnText = getKeyTextTranslation(sBtnText);	
		
		sBtnLabel = QString("%1%2").arg(IHM_LABEL_DESC_BTN_ID).arg(iKeyID);
		sBtnDescription = getKeyTextTranslation(sBtnLabel);
		if(sBtnDescription == sBtnLabel)
			sBtnDescription = sBtnText;

		// Here we need to update "ActionList" with data of pressed button.
		sBtnLbl = QString("[%1] : %2 - %3").arg(sDetectedString).arg(getKbdSpecialKeyTypeLabel()).arg(sBtnDescription);
		
		updateMsgList(sBtnLbl);
	}

	
}

void MFormTestKeyboard::onStringDetected(QString sCoupleID, QString sDetectedString)
{
	QString sBtnText, sBtnLbl;
	QWidget * pCurrentWidget = NULL;
	bool bValidString = false;
	
	
	// We find object that is just pressed and we need to display it as pressed.
	MFieldObjButton * pDetectedObj = MFieldObjButton::findFieldObjectByKeyID(&m_lstFieldObjList, sCoupleID);
	

	while(pDetectedObj != NULL) 
	{
		if(!pDetectedObj->isReadOnly())
		{
			pCurrentWidget = pDetectedObj->getWidget();
			((QAbstractButton*)pCurrentWidget)->animateClick();
			bValidString = true;
		}

		pDetectedObj = MFieldObjButton::findNextFieldObjectByKeyID(pDetectedObj, &m_lstFieldObjList, sCoupleID);
	}

	if(bValidString)
	{

		QString sBtnLabel = QString("%1%2").arg(IHM_LABEL_DESC_BTN_ID).arg(sCoupleID);
		QString sBtnDescription = getKeyTextTranslation(sBtnLabel);
		QString sOut; 

//		sRegExStr = "^[\?]?[4|5][\d]{5}([\d]*)([=])([\d]{6})[.]*";
		

		if(m_bFilterDisplayedStr)		
		{
			maskDetectedString(sDetectedString, sOut);
			sBtnLbl = QString("[%1] : %2").arg(sOut).arg(sBtnDescription);
		}
		else
		{
			// Here we need to update "ActionList" with data of pressed button.	
			sBtnLbl = QString("[%1] : %2").arg(sDetectedString).arg(sBtnDescription);
		}

		updateMsgList(sBtnLbl);
	}
}


void MFormTestKeyboard::maskDetectedString(const QString &sInput, QString &sOutput)
{
	int iLen = sInput.length();

	QRegularExpressionMatch match = m_rxMaskDetectedString.match(sInput);

	if(match.hasMatch())
		{
			QStringList sCapList = match.capturedTexts();
			QString sTemp;
			int iCurrentPos, iPosCap;
			QString sCurrentCap;
			int iCurrentCapLen;
			
			sOutput="";
			iCurrentPos = 0;

			//captured grops start from 1 since the 0 is the complete result
			for (int i=1; i<sCapList.size();i++)
			{
				sCurrentCap = sCapList.at(i);
				iPosCap = sInput.indexOf(sCurrentCap);
				iCurrentCapLen = sCurrentCap.size();
				
				sOutput.append(QString(iPosCap - iCurrentPos, m_chReplacementChar));
				sOutput.append(sCurrentCap);			
				iCurrentPos = iPosCap + iCurrentCapLen;
			}

			sOutput.append(QString(iLen - iCurrentPos, m_chReplacementChar));


		}
	else
		sOutput = sInput;
		
}




bool MFormTestKeyboard::initialize(MInputDialogExReq * pReq)
{
 	QString sInputTemplate;

	m_oReq = *pReq;

	sInputTemplate = pReq->getDskUITemplate();

	if(sInputTemplate!="")
	{
		QString sTemplatePath =  MIhmConfigGeneral::getCfg()->getUIFullPath(sInputTemplate);

		if(!loadTemplate(sTemplatePath))
		{
			TRACE_W(QString("MFormTestKeyboard::initialize: template not loaded!"));
			return false;
		}

		m_bTemplateLoaded = true;
	}
	else
	{
		TRACE_W(QString("MFormTestKeyboard::initialize: No UI template file defined for AboutDialogTemplate!"));
		return false;
	}
	
	installEventFilter(m_pParentWindow);
	//connect defined objects on form
	if(connectDefinedObjects())
	{
		m_bTemplateLoaded = true;
	}

	m_chReplacementChar = MIhmConfigGeneral::getCfg()->getStrDetMaskChar();

	//initialize regex for bank string detection masking
	QString sRegExPatern = MIhmConfigGeneral::getCfg()->getStrDetMaskRegex();

	if(!sRegExPatern.isEmpty())
	{
		m_rxMaskDetectedString.setPattern(sRegExPatern);		
		m_bFilterDisplayedStr = m_rxMaskDetectedString.isValid();
	}
	else
		m_bFilterDisplayedStr = false;

	if(m_bFilterDisplayedStr)
		m_bFilterDisplayedStr =  MIhmConfigGeneral::getCfg()->getStrDetMaskEnabled();

	return m_bTemplateLoaded;
}


void MFormTestKeyboard::focusFirstInput()
{
	//NOP
}

void MFormTestKeyboard::initMouseCursorInitPos()
{
	//NOP
}

bool MFormTestKeyboard::keyPressEventHandler(QObject *obj, QKeyEvent * event)
{
	return MInputExFiledsKbdInterface::keyPressEventHandler(obj, event);
}




void MFormTestKeyboard::pressedKeyValidate()
{	
	//NOP
}

void MFormTestKeyboard::pressedKeyCancel()
{
	onButtonCancel();
}

void MFormTestKeyboard::onButtonCancel()
{
	if(doCancel())
		emit rejected();
}

bool MFormTestKeyboard::doValidate()
{
	return true;
}

bool MFormTestKeyboard::doCancel()
{
	m_sResult = "";
	return true;
}

void MFormTestKeyboard::show()
{
	MFormInputEx::show();

}

void MFormTestKeyboard::updateMsgList(QString sListItemText)
{
	QListWidgetItem *newItem = new QListWidgetItem;
	newItem->setText(sListItemText);
	
	m_pListWidget->addItem(newItem);

	if(m_pListWidget->count()>20)
		delete m_pListWidget->takeItem(1);

	newItem = m_pListWidget->item(m_pListWidget->count()-1);
	m_pListWidget->scrollToItem(newItem); 
}


QString MFormTestKeyboard::getKbdSpecialKeyTypeLabel()
{
	QString sRetVal;
	MIhmConfigGeneral::getCfg()->getLanguages()->getLabelTranslation(IHM_LABEL_KBD_SPECIAL_KEY, MIhmLanguages::enuTranslTargetDesktop, sRetVal);
	return sRetVal;
}

QString MFormTestKeyboard::getKbdKeyTypeLabel()
{
	QString sRetVal;
	MIhmConfigGeneral::getCfg()->getLanguages()->getLabelTranslation(IHM_LABEL_KBD_KEY, MIhmLanguages::enuTranslTargetDesktop, sRetVal);
	return sRetVal;
}

QString MFormTestKeyboard::getKeyboardTestTitleLabel()
{
	QString sRetVal;
	MIhmConfigGeneral::getCfg()->getLanguages()->getLabelTranslation(IHM_LABEL_KBD_TEST_TITLE, MIhmLanguages::enuTranslTargetDesktop, sRetVal);
	return sRetVal;
}

QString MFormTestKeyboard::getKeyTextTranslation(QString sConfLabelName)
{
	QString sRetVal;
	MIhmConfigGeneral::getCfg()->getLanguages()->getLabelTranslation(sConfLabelName, MIhmLanguages::enuTranslTargetDesktop, sRetVal);
	return sRetVal;
}


