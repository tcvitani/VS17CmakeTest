
#include "MHelpFuncs.h"
#include "MDskAsyncInputs.h"
#include "MInputExFieldObj.h"
#include <QWidget>
#include <QEvent>
#include <QKeyEvent>
#include "MTracer.h"


extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
	#include "ihm_loc.h"
}
 

MDskAsyncInputs::MDskAsyncInputs(QWidget * pParentForm)
{
	m_pParentForm = pParentForm;
}


MDskAsyncInputs::~MDskAsyncInputs()
{
	while (!m_lstFieldObjList.isEmpty())
		delete m_lstFieldObjList.takeFirst();
}




bool MDskAsyncInputs::createAndConnectDefinedObjects(MInputAsyncExReq * pReq)
{
	if(m_dynFormWidget==NULL)
	{
		TRACE_W(QString("MDskAsyncInputs::createAndConnectDefinedObjects: m_dynFormWidget==NULL")); 
		return false;
	}

	while (!m_lstFieldObjList.isEmpty())
		delete m_lstFieldObjList.takeFirst();
	
	
	QList <MInputExField *> * plstFieldList = pReq->getInputFields();

	MInputExField * pCurrentField;

	for (int i=0; i<plstFieldList->size();i++)
	{
		pCurrentField = plstFieldList->at(i);
		
		MInputExFieldObj * pNewObj = MInputExFieldObj::createNewFieldObj(pCurrentField);
		
		if(pNewObj!=NULL)
		{
			if(pNewObj->connectToWidget(m_dynFormWidget, pCurrentField))
			{
				m_lstFieldObjList.append(pNewObj);
				
				connect(pNewObj, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));

				//install the main event filter
				pNewObj->getWidget()->installEventFilter(this);
				pNewObj->getWidget()->installEventFilter(m_pParentForm);

			}
			else
				delete pNewObj;
		}
		else
		{
			TRACE_W(QString("MDskAsyncInputs::createAndConnectDefinedObjects: Unknown object type for object %1!")
								.arg(pCurrentField->getName()));

			return false;
		}

	}

	return true;
}



bool MDskAsyncInputs::initialize(MInputAsyncExReq * pReq, QWidget *dynFormWidget)
{	
	TRACE_D(QString("MDskAsyncInputs::initialize: definition:%1")
								.arg(pReq->getDefinition()));
	
	//keep the pointer to the objects loaded from the main form ui
	m_dynFormWidget = dynFormWidget;

	if(!m_bInitialized)
	{
		m_bInitialized = createAndConnectDefinedObjects(pReq);

		setCorrectTabOrdering();
	}

	return m_bInitialized;
}



bool MDskAsyncInputs::updateValues(MInputAsyncExReq * pValReq)
{
	TRACE_D(QString("MDskAsyncInputs::updateValues:... "));

	QList <MInputExField *> * plstFieldList = pValReq->getInputFields();

	MInputExField * pCurrentField;

	for (int i=0; i<plstFieldList->size();i++)
	{
		pCurrentField = plstFieldList->at(i);
		MInputExFieldObj * pFieldObj = 
			MInputExFieldObj::findFieldObject(&m_lstFieldObjList,pCurrentField->getName());
		
		if(pFieldObj!=NULL)
			if(pFieldObj->getType() == pCurrentField->getType())
			{
				pFieldObj->updateData(pCurrentField);
			}
	}

	focusFirstInput();

	return true;
}


QString MDskAsyncInputs::getEncStrValues()
{
	QString sRetVal;
	sRetVal += IHM_SAISIE_EX_SEPARATEUR_CHAMP;

	MInputExFieldObj * pCurrentFieldObj;

	for (int i=0; i<m_lstFieldObjList.size();i++)
	{
		pCurrentFieldObj = 	m_lstFieldObjList.at(i);

		sRetVal += pCurrentFieldObj->getEncStrValue();
		sRetVal += IHM_SAISIE_EX_SEPARATEUR_CHAMP;
	}
	
	return sRetVal;
}


//inplemetnation of MDlgKbdInterface 
bool MDskAsyncInputs::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::KeyPress) 
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		return keyPressEventHandler(obj, keyEvent);
	}
	else {
		 // standard event processing
		 return QObject::eventFilter(obj, event);
	}
}


void MDskAsyncInputs::pressedKeyValidate()
{
	;// do nothing , TODO or should we close the comment tab???
}

void MDskAsyncInputs::pressedKeyCancel()
{
	;// do nothing , TODO or should we close the comment tab???
}


void MDskAsyncInputs::initMouseCursorInitPos()
{
	;// do nothing since it is not used for comment inputs
}