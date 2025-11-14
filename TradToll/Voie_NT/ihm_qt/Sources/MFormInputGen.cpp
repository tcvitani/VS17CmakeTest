
#include <QtUiTools>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QFrame>
#include <QKeyEvent>
#include <QJSEngine>
#include <QRect>
#include <QPushButton>

#include "MIhmConfigGeneral.h"
#include "MFormInputGen.h"
#include "MIhmLanguages.h"
#include "MCtlEdit.h"
#include "MCtlList.h"
#include "MHelpFuncs.h"
#include "MInputExFieldObj.h"
#include "MTracer.h"


extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}
 

MFormInputGen::MFormInputGen(QWidget *pParentWindow)
    : MFormInputEx(pParentWindow)
{

	setWindowFlags(Qt::Dialog
						| Qt::MSWindowsFixedSizeDialogHint
						| Qt::FramelessWindowHint);

	m_bTemplateLoaded = false;
	m_sResult = "";
	m_bOkButton = false;
	m_bCancelButton = false;
	m_pFieldObjButtonOK = NULL;

}


MFormInputGen::~MFormInputGen()
{
	while (!m_lstFieldObjList.isEmpty())
		delete m_lstFieldObjList.takeFirst();
}


void MFormInputGen::initMouseCursorInitPos()
{
	MInputExFieldObj * pCurrentObj;

	for (int i=0; i<m_lstFieldObjList.size();i++)
	{
		pCurrentObj = m_lstFieldObjList.at(i);

		if(pCurrentObj->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON_OK || 
			pCurrentObj->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON_CANCEL )
		{
			QWidget * w = pCurrentObj->getWidget();
		
			if(w!=NULL)
			{	
				QPoint pt = w->pos();

				m_iInitMousePosX = pt.x() + (w->width()/2);
				m_iInitMousePosY = pt.y() + (w->height()/2);

				if(pCurrentObj->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON_OK)
					break;
			}

		}	
	}

}

bool MFormInputGen::loadTemplate(QString sUIFilePath)
{
	bool bRetVal = false;
	QUiLoader loader;

	TRACE_D(QString("MFormInputEx::loadTemplate: sUIFilePath:%1")
									.arg(sUIFilePath));
	
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
			bRetVal = true;
		}
		else
		{
			TRACE_W(QString( "MFormInputEx::loadTemplate: Error loading %1! The main container QFrame name frContainer not found in the template!")
									.arg(sUIFilePath));
		}

	}
	
	if(!bRetVal)
	{
		TRACE_W(QString( "MFormInputEx::loadTemplate: Error loading %1")
									.arg(sUIFilePath));
		
	}

	return bRetVal;
}




//Get field definition list from the request and try to find the correct widgets, 
// in the dynamically loaded form (form ui file), to get its pointer and update the values
// and connect signals from it
bool MFormInputGen::connectDefinedObjects()
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
				if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON_OK)
				{
					QWidget * w = pNewObj->getWidget();
					
					if(w!=NULL)
					{
						m_bOkButton = true;
						QObject::connect(w, SIGNAL(clicked()), this, SLOT(onButtonOK()));
						m_pFieldObjButtonOK = pNewObj;
					}
				}	
				else if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON_CANCEL)
				{
					m_bCancelButton = true;
					QWidget * w = pNewObj->getWidget();
					
					if(w!=NULL)
						QObject::connect(w, SIGNAL(clicked()), this, SLOT(onButtonCancel()));
				}
				else if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_LABEL_LUHN_CHECK)
				{
					//TODO init Luhn Key check images
					MIhmConfigImages * pImages = MIhmConfigGeneral::getCfg()->getDskConfigImages();

					QString sImgLuhnOK = pImages->getLuhnCheckOKImgPath();
					QString sImgLuhnNOK = pImages->getLuhnCheckNOKImgPath();

					((MFieldObjLuhnCheckLabel*)pNewObj)->initLuhnKeyIcons(sImgLuhnOK, sImgLuhnNOK);
					((MFieldObjLuhnCheckLabel*)pNewObj)->updateLuhnKeyCheckIcon(false);
				}	

				m_lstFieldObjList.append(pNewObj);
				
				//install the main event filter
				pNewObj->getWidget()->installEventFilter(this);
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





bool MFormInputGen::initialize(MInputDialogExReq * pReq)
{	
	//keep the pointer to 
	m_oReq = *pReq;

	TRACE_D(QString("MFormInputGen::initialize: dialog_id:%1")
									.arg(m_oReq.getDialogId()));

	QString sInputTemplate = pReq->getDskUITemplate();
	QString sTemplatePath = MIhmConfigGeneral::getCfg()->getUIFullPath(sInputTemplate);

	if(!loadTemplate(sTemplatePath))
	{
		TRACE_W(QString("MFormInputGen::initialize: template %1 not loaded!")
									.arg(sTemplatePath));
		return false;
	}

	m_bTemplateLoaded = connectDefinedObjects();

	if(!m_bTemplateLoaded)
	{
		TRACE_W(QString("MFormInputGen::initialize: connectDefinedObjects returned false for file %1!")
									.arg(sTemplatePath));
		return false;
	}

	//set additional style sheets
	QString sCSSFilePath = m_oReq.getDskCSSFile();
	if(sCSSFilePath!="")
	{
		sCSSFilePath = MIhmConfigGeneral::getCfg()->getCSSFullPath(sCSSFilePath);
		MHelpFuncs::setFileCSSToWidget(sCSSFilePath,this);
	}
	
	m_bInitialized = initFieldFormatHandling();
	
	initFieldFocusNextHandling();

	updateCalcualtedValues();
	onDataChanged();
	
	return m_bInitialized;
}



bool MFormInputGen::initFieldFormatHandling()
{
	TRACE_D(QString("MFormInputGen::initFieldFormatHandling: %1")
									.arg(m_oReq.getDialogId()));

	MInputExFieldObj * pCurrentObj;

	for (int i=0; i<m_lstFieldObjList.size();i++)
	{
		pCurrentObj = m_lstFieldObjList.at(i);
		
		//Connect signal slot for the fields that needs to be updated on change 
		// - line edits with FILTER format to update referenced list widget. It is using set filter)
		// - enuFIELD_FORMAT_LUHN_KEY is using updateLuhnKeyCheckIcon
		// all others are re-validating form content to enable/disable btnOK 
		if(pCurrentObj->getType()== MInputExField::enuDLG_EX_FIELD_LINE_EDIT || 
			pCurrentObj->getType()== MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT ||
			pCurrentObj->getType()== MInputExField::enuDLG_EX_FIELD_COMBO_BOX ||
			pCurrentObj->getType()== MInputExField::enuDLG_EX_FIELD_LIST_WIDGET||
			pCurrentObj->getType()== MInputExField::enuDLG_EX_FIELD_TEXT_EDIT)
		{
			QObject::connect(pCurrentObj,SIGNAL(dataChanged()), this, SLOT(onDataChanged()));
		}

	}
		
	return true;
}

void MFormInputGen::initFieldFocusNextHandling()
{
	TRACE_D(QString("MFormInputGen::initFieldFocusNextHandling: %1")
		.arg(m_oReq.getDialogId()));

	MInputExFieldObj * pCurrentObj;

	for (int i = 0; i < m_lstFieldObjList.size(); i++)
	{
		pCurrentObj = m_lstFieldObjList.at(i);


		if (pCurrentObj->getType() == MInputExField::enuDLG_EX_FIELD_LINE_EDIT ||
			pCurrentObj->getType() == MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT)
		{
			QObject::connect(pCurrentObj, SIGNAL(focusNext()), this, SLOT(onFocusNextReq()));
		}

	}

}


void MFormInputGen::onFocusNextReq()
{
	MInputExFieldObj * pSender = qobject_cast<MInputExFieldObj *>(sender());

	if (pSender != NULL)
	{
		if (pSender->getType() == MInputExField::enuDLG_EX_FIELD_LINE_EDIT ||
			pSender->getType() == MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT)
		{
			moveFocusToNextInput();
		}
	}
}



void MFormInputGen::onDataChanged()
{
    MInputExFieldObj * pSender = qobject_cast<MInputExFieldObj *>(sender());

	if(pSender!=NULL)
	{
		if((pSender->getType()== MInputExField::enuDLG_EX_FIELD_LINE_EDIT || 
			pSender->getType()== MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT) &&
			(pSender->getFormatType()== MInputExField::enuFIELD_FORMAT_FILTER ||
			pSender->getFormatType() == MInputExField::enuFIELD_FORMAT_FILTER_BEGINS_WITH))
		{	
			//to update filtered list ...
			QString sListToFilterName = pSender->getFormatDetail();
			QString sFilterValue = ((MFieldObjLineEdit*)pSender)->getCurrentValue();

			//extract the name of the list to be filtered
			if(sListToFilterName!="")
			{
				//find the pointer to object
				MInputExFieldObj * pList = findFieldObjectByName(sListToFilterName);

				//if found and if it is a QListWidget 
				//- set the filter string to the list (the list should auto update its content)
				if (pList != NULL && pList->getType() == MInputExField::enuDLG_EX_FIELD_LIST_WIDGET)
				{
					MFieldObjListWidget::enuFilterType eFilterType = MFieldObjListWidget::enuFILTER_ANYWHERE;
					if (pSender->getFormatType() == MInputExField::enuFIELD_FORMAT_FILTER_BEGINS_WITH)
						eFilterType = MFieldObjListWidget::enuFILTER_BEGINS_WITH;

					((MFieldObjListWidget*)pList)->setFilter(sFilterValue, eFilterType);
				}
			}
		}
		else if(pSender->getType()== MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT &&
			(pSender->getFormatType()== MInputExField::enuFIELD_FORMAT_LUHN_KEY || 
			pSender->getFormatType()== MInputExField::enuFIELD_FORMAT_LUHN_KEY_MOD16||
			pSender->getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY_MOD16_M3))
		{	//to update luhn key label
			QString sLuhnKeyCheckLabel = pSender->getFormatDetail();
			bool bLuhnOK = ((MFieldObjLineEditEx*)pSender)->isValidValue();

			//extract the name of the label used to update the image
			if(sLuhnKeyCheckLabel!="")
			{
				//find the pointer to object
				MInputExFieldObj * pLuhnLabel = findFieldObjectByName(sLuhnKeyCheckLabel);

				//if found and if it is a QListWidget 
				//- set the filter string to the list (the list should auto update its content)
				if(pLuhnLabel!=NULL && pLuhnLabel->getType()==MInputExField::enuDLG_EX_FIELD_LABEL_LUHN_CHECK)
				{
					((MFieldObjLuhnCheckLabel*)pLuhnLabel)->updateLuhnKeyCheckIcon(bLuhnOK);
				}
			}
		}
		
		if(pSender->getType()== MInputExField::enuDLG_EX_FIELD_LINE_EDIT || 
			pSender->getType()== MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT)
					updateCalcualtedValues();
	}

	//in all cases re-validate all fields to update the look of the dialog
	doValidate(false);
}




void MFormInputGen::updateCalcualtedValues()
{

	MInputExFieldObj * pCurrentObj;

	for (int i=0; i<m_lstFieldObjList.size();i++)
	{
		pCurrentObj = m_lstFieldObjList.at(i);

		if(pCurrentObj->getType()== MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT && 
			pCurrentObj->getFormatType()== MInputExField::enuFIELD_FORMAT_CALCULATION)
		{
			QString sCurrentObjName = pCurrentObj->getName();
			int iResultPrecision = pCurrentObj->getFieldData()->getResultPrecision();

			QString sFormula = pCurrentObj->getFormatDetail(); //format detail for calculation is a formula
			QString sTextToSet = calculateEvaluation(sFormula, sCurrentObjName,  iResultPrecision);
			
			if(pCurrentObj->getFieldData()->isLblSwitchingEnabled())
			{
				sFormula = pCurrentObj->getFieldData()->getLbLSwitchingFormula();
				QString sResult = calculateEvaluation(sFormula, sCurrentObjName,  0); //send 0 for the precision to avoid changing of result

				QString sLabelFalseName = pCurrentObj->getFieldData()->getLbLSwitchingLabelFalse();
				QString sLabelTrueName = pCurrentObj->getFieldData()->getLbLSwitchingLabelTrue();
				
				MInputExFieldObj * pFieldObjLblFalse = findFieldObjectByName(sLabelFalseName);
				MInputExFieldObj * pFieldObjLblTrue = findFieldObjectByName(sLabelTrueName);
				
				if(pFieldObjLblFalse==NULL 
						|| pFieldObjLblTrue==NULL 
						|| pFieldObjLblFalse->getType()!=MInputExField::enuDLG_EX_FIELD_LABEL
						|| pFieldObjLblTrue->getType()!=MInputExField::enuDLG_EX_FIELD_LABEL)
				{
					TRACE_W(QString("MFormInputGen::updateCalcualtedValues: Error getting label switching target labels %1 and %2!")
									.arg(sLabelFalseName)
									.arg(sLabelTrueName));
				}
				else
				{
					if(sResult=="true")
					{
						//show label_true and hide label_false
						pFieldObjLblFalse->getWidget()->setVisible(false);
						pFieldObjLblTrue->getWidget()->setVisible(true);
					}
					else
					{
						//show label_false and hide label_true
						pFieldObjLblFalse->getWidget()->setVisible(true);
						pFieldObjLblTrue->getWidget()->setVisible(false);
					}
				}

			}

			((MFieldObjLineEditEx*)pCurrentObj)->updateValue(sTextToSet);
		}

	}
	
}


QString MFormInputGen::calculateEvaluation(QString sFormula, QString sSkipObjectName, int iResultPrecision)
{
	QString sResult;
	QString sEval = sFormula;

	MInputExFieldObj * pCurrentObj;
	QString sName;
	QString sValue;
	QChar cDefDecimalSeparator = MIhmConfigGeneral::getCfg()->getDefaultDecimalSeparator();

	for(int i=0; i<m_lstFieldObjList.size();i++)
	{
		pCurrentObj = m_lstFieldObjList.at(i);
		
		if(pCurrentObj->getType()!= MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT && 
			pCurrentObj->getType()!= MInputExField::enuDLG_EX_FIELD_LINE_EDIT &&
			pCurrentObj->getType()!= MInputExField::enuDLG_EX_FIELD_HIDDEN_EDIT)
			continue;

		sName = pCurrentObj->getName();

		if(sSkipObjectName==sName)
			continue;

		sValue = "";

		if(pCurrentObj->getType()== MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT )
		{
			sValue = ((MFieldObjLineEditEx*)pCurrentObj)->getCurrentValue();
		}
		else //for enuDLG_EX_FIELD_LINE_EDIT and enuDLG_EX_FIELD_HIDDEN_EDIT
		{
			sValue = ((MFieldObjLineEdit*)pCurrentObj)->getCurrentValue();
		}

		sValue = sValue.replace(cDefDecimalSeparator,'.');

		sEval = sEval.replace(sName,sValue);
	}

	QJSEngine myEngine;
	QJSValue result = myEngine.evaluate(sEval);
	
	if(!myEngine.hasError()){
		
		sResult = result.toString();

		if(iResultPrecision>0)
		{
			bool bOK;
			double d = sResult.toDouble(&bOK);
			if (bOK)
			{
				sResult = QString::number(d, 'f', iResultPrecision);
				sResult = sResult.replace('.', cDefDecimalSeparator);
			}
		}
	}
	else
	{
		sResult = "#ERR";
		TRACE_D(QString("MFormInputGen::calculateEvaluation: Error evaluating:%1!")
									.arg(sEval));
	}


	return sResult;
}


//To be able to know when to focus to the invalid field we (if VALID button is always enabled by parameter)
//need to send if the event was triggered automatically (bTriggeredByButton==FALSE),
// or by pressing VALID button (bTriggeredByButton==FALSE)
bool MFormInputGen::doValidate(bool bTriggeredByButton)
{

	if(!m_bOkButton)
		return false;
	
	bool bResult = true;

	QWidget *pFirstInvalidWidget = NULL;

	QString sRes;
	
	MInputExFieldObj * pCurrentObj;

	for (int i=0; i<m_lstFieldObjList.size();i++)
	{
		pCurrentObj = m_lstFieldObjList.at(i);
		
		if(pCurrentObj!=NULL)
		{
			if(pCurrentObj->retunsValue())
				if(pCurrentObj->isValidValue())
				{
					if(sRes=="")
						sRes = IHM_SAISIE_EX_SEPARATEUR_CHAMP;
						
					QString sValue = pCurrentObj->getEncStrValue();
					sRes += sValue;
					sRes += IHM_SAISIE_EX_SEPARATEUR_CHAMP;
					
					pCurrentObj->getWidget()->setStyleSheet("");
				}
				else
				{
					pCurrentObj->getWidget()->setStyleSheet(MIhmConfigGeneral::getCfg()->getInputDlgsErrorFieldsStyle()); 
					bResult = false;

					if(pFirstInvalidWidget==NULL)
						pFirstInvalidWidget = pCurrentObj->getWidget();
				}
		}	
	}	

	if(bResult)
		m_sResult = sRes;
	else if(bTriggeredByButton)
	{
		if(pFirstInvalidWidget!=NULL)
			pFirstInvalidWidget->setFocus();
	}

	//update the btnOK state if not configured to be always enabled
	if(!MIhmConfigGeneral::getCfg()->getInputDlgsValidAlwaysEnabled())
 		if(!m_pFieldObjButtonOK->isReadOnly()) 

 		{
 			QWidget * w = m_pFieldObjButtonOK->getWidget();
 			w->setEnabled(bResult);
 		}

	return bResult;
}


bool MFormInputGen::doCancel()
{
	if(m_bCancelButton)
		m_sResult = "";

	return m_bCancelButton; //return true if cancel button exists
}


void MFormInputGen::pressedKeyValidate()
{
	onButtonOK();
}

void MFormInputGen::pressedKeyCancel()
{
	onButtonCancel();
}

void MFormInputGen::onButtonOK()
{
	if(doValidate())
		emit accepted();
}

void MFormInputGen::onButtonCancel()
{
	if(doCancel())
		emit rejected();
}
