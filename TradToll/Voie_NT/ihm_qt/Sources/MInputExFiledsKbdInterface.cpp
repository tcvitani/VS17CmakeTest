

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QCursor>
#include <QApplication>
#include <QWidget>
#include <QPushButton>

#include "MInputExFiledsKbdInterface.h"
#include "MInputExFieldObj.h"


 

MInputExFiledsKbdInterface::MInputExFiledsKbdInterface()
{
	m_iInitMousePosX = -1;
	m_iInitMousePosY = -1;
	m_bInitialized = false;

}


MInputExFiledsKbdInterface::~MInputExFiledsKbdInterface()
{

}




MInputExFieldObj * MInputExFiledsKbdInterface::findFieldObjectByName(QString sName)
{
	
	MInputExFieldObj * pCurrentObj;

	for (int i=0; i<m_lstFieldObjList.size();i++)
	{
		pCurrentObj = m_lstFieldObjList.at(i);

		if(pCurrentObj->getName() == sName)
			return pCurrentObj;
	}

	return NULL;
}



MInputExFieldObj * MInputExFiledsKbdInterface::findFieldObject(QObject *pWidget)
{
	//Find the field object and get its type
	MInputExFieldObj * pCurrentObj = NULL;

	for (int i=0; i<m_lstFieldObjList.size();i++)
	{
		pCurrentObj = m_lstFieldObjList.at(i);
		
		if(pCurrentObj!=NULL)
		{
			if((QObject *)pCurrentObj->getWidget()==pWidget)
				return pCurrentObj;
		}
	}

	return NULL;
}

void MInputExFiledsKbdInterface::focusFirstInput()
{
	//set the focus to the first available input control which returs true in canGetFocus 
	MInputExFieldObj * pNext = findNextFieldObjectToFocus(NULL);
	if(pNext!=NULL)
	{
		QWidget * w = pNext->getWidget();
		if(w!=NULL)			
			w->setFocus();
	}

}


MInputExFieldObj * MInputExFiledsKbdInterface::findNextFieldObjectToFocus(MInputExFieldObj * pCurrentInFocus)
{
	//Find the field object and get its type
	MInputExFieldObj * pCurrentObj = NULL;
	MInputExFieldObj * pObjToFocus = NULL;
	bool bCurrentFound = false;

	for (int i=0; i<m_lstFieldObjList.size();i++)
	{
		pCurrentObj = m_lstFieldObjList.at(i);
		
		if(pCurrentInFocus==NULL) // used for find first
		{
			if(pCurrentObj!=NULL && pCurrentObj->canGetFocus())
			{
				pObjToFocus = pCurrentObj;
				break;
			}
		}
		else if(!bCurrentFound && pCurrentObj==pCurrentInFocus)
		{
			bCurrentFound = true;
			continue;
		}
		else if(bCurrentFound && pCurrentObj!=NULL && pCurrentObj->canGetFocus())
		{
			pObjToFocus = pCurrentObj;
			break;
		}
	}

	if(pObjToFocus == NULL && bCurrentFound) //if it is the last in the list find first to focus
		pObjToFocus = findNextFieldObjectToFocus(NULL);
	
	return pObjToFocus;

}


MInputExFieldObj * MInputExFiledsKbdInterface::findPreviousFieldObjectToFocus(MInputExFieldObj * pCurrentInFocus)
{
	//Find the field object and get its type
	MInputExFieldObj * pCurrentObj = NULL;
	MInputExFieldObj * pPreviousObj = NULL;
	MInputExFieldObj * pObjToFocus = NULL;

	bool bCurrentFound = false;

	for (int i = 0; i < m_lstFieldObjList.size(); i++)
	{
		if (pCurrentObj != NULL && pCurrentObj->canGetFocus())
			pPreviousObj = pCurrentObj; //memorize one before which can get focus
		
		pCurrentObj = m_lstFieldObjList.at(i);

		if (pCurrentInFocus == NULL) // used for find first
		{
			if (pCurrentObj != NULL && pCurrentObj->canGetFocus())
			{
				pObjToFocus = pCurrentObj;
				break;
			}
		}
		else if (!bCurrentFound && pCurrentObj == pCurrentInFocus)
		{
			bCurrentFound = true;
			pObjToFocus = pPreviousObj;
			break;
		}
	}

	if (pObjToFocus == NULL && bCurrentFound) //if it was the last in the list, find first to focus
		pObjToFocus = findPreviousFieldObjectToFocus(NULL);

	return pObjToFocus;

}

bool MInputExFiledsKbdInterface::keyPressEventHandler(QObject *obj, QKeyEvent * event)
{
	//Find the field object and get its type
	MInputExFieldObj * pFieldObj = NULL;
	
	if(m_bInitialized)	
		pFieldObj = findFieldObject(obj);

	int iKey = event->key();

	if(iKey == Qt::Key_Enter|| iKey == Qt::Key_Return) //special handling of ENTER key
	{
		if (pFieldObj != NULL)
		{
			// if button execute the button
			//else move to the next input 
			if (pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON_OK ||
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON_CANCEL ||
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON)
			{
				QWidget * w = pFieldObj->getWidget();

				QPushButton *pButton = static_cast<QPushButton *>(w);
				if (pButton != NULL)
					pButton->click();
			}
			else if (pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_TEXT_EDIT)
			{
				//Note that text edit is specific since it can use enter for new line character
				if (pFieldObj->isReadOnly())
				{
					moveFocusToNextInput();
				}
				else
					return false; //meaning do not filter this event so the text edit can handle it by itself
			}
			else if (pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_COMBO_BOX)
			{
				moveFocusToNextInput();
				return false;
			}
			else
				moveFocusToNextInput();//move focus down

			return true;
		}
		
	}
	else if (iKey == Qt::Key_Down ) //special handling of DOWN arrow key
	{
		if (pFieldObj != NULL)
		{
			if(pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_LINE_EDIT || 
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT || 
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON_OK ||
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON_CANCEL ||
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON)
			{
				moveFocusToNextInput();//move focus down
				return true;
			}

			return false; //meaning do not filter this event so the input ctrl can handle it by itself
		}

	}
	else if (iKey == Qt::Key_Up) //special handling of UP arrow key
	{
		if (pFieldObj != NULL)
		{
			if (pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_LINE_EDIT ||
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT ||
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON_OK ||
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON_CANCEL ||
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON)
			{
				moveFocusToPreviousInput();//move focus down
				return true;
			}

			return false; //meaning do not filter this event so the input ctrl can handle it by itself
		}

	}
	else if(iKey == Qt::Key_Space) //disable button click on space key 
	{
		//ignore if button
		if (pFieldObj != NULL)
		{
			if (pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON_OK ||
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON_CANCEL ||
				pFieldObj->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON)
				return true;
		}
	}
	else if(iKey == Qt::Key_Escape)
	{
		pressedKeyCancel();
		return true;
	}

	return false;
}




void MInputExFiledsKbdInterface::setCorrectTabOrdering()
{
	MInputExFieldObj * pCurrentObj;
	QWidget * pPreviousFocusObj = NULL;
	QWidget * pCurrentWidget = NULL;

	for (int i=0; i<m_lstFieldObjList.size();i++)
	{
		pCurrentObj = m_lstFieldObjList.at(i);
		
		pCurrentWidget = pCurrentObj->getWidget();
		
		if(pCurrentWidget!=NULL)
			if(pCurrentWidget->focusPolicy()!= Qt::NoFocus)
			{
				if(pPreviousFocusObj!=NULL)
				{
					 QWidget::setTabOrder(pPreviousFocusObj, pCurrentWidget);
				}

				pPreviousFocusObj = pCurrentWidget;	
			}
	}

}




void MInputExFiledsKbdInterface::moveFocusToNextInput()
{
	QWidget *pInput = QApplication::focusWidget();
	
	MInputExFieldObj * pCurrent = findFieldObject(pInput);
	
	if(pCurrent!=NULL)
	{
		MInputExFieldObj * pNext = findNextFieldObjectToFocus(pCurrent);
		if(pNext!=NULL)
		{
			QWidget * w = pNext->getWidget();
			if(w!=NULL)			
				w->setFocus();
		}
	
	}
	

}


void MInputExFiledsKbdInterface::moveFocusToPreviousInput()
{
	QWidget *pInput = QApplication::focusWidget();

	MInputExFieldObj * pCurrent = findFieldObject(pInput);

	if (pCurrent != NULL)
	{
		MInputExFieldObj * pNext = findPreviousFieldObjectToFocus(pCurrent);
		if (pNext != NULL)
		{
			QWidget * w = pNext->getWidget();
			if (w != NULL)
				w->setFocus();
		}

	}


}

