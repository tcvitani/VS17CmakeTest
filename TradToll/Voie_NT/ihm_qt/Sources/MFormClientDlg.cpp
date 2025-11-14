
#include <QKeyEvent>
#include <QPushButton>

#include "MIhmConfigGeneral.h"
#include "MFormClientDlg.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}


MFormClientDlg::MFormClientDlg(QWidget *parent)
    : QWidget(parent)
{
	setWindowFlags(Qt::Dialog
						| Qt::MSWindowsFixedSizeDialogHint
						| Qt::FramelessWindowHint);

	setWindowModality(Qt::ApplicationModal);

	m_bTemplateLoaded = false;
	m_pParentWindow = parent;

	if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
		this->setCursor(Qt::BlankCursor);
	else
		this->setCursor(Qt::ArrowCursor);

}

MFormClientDlg::~MFormClientDlg()
{

}

void MFormClientDlg::show()
{

	QPoint ptWinPos;
	//Show only if template correctly loaded
    if(m_bTemplateLoaded)
	{

		//dialog alignment
		if(m_sEmbedTo=="")
		{
			QPoint ptMainWinPos = m_pParentWindow->pos();
			QSize siSize = m_pParentWindow->size();
			
			ptWinPos = ptWinPos + QPoint((siSize.width()/2) - (this->width()/2), (siSize.height()/2) - (this->height()/2));

			this->move(ptWinPos.x(),ptWinPos.y());
		}
		else //	if(m_sEmbedTo!="")
		{

			QWidget * pEmbedingWidget = m_pParentWindow->findChild<QWidget *>(m_sEmbedTo);
			
			if(pEmbedingWidget!=NULL)
			{	
				ptWinPos = m_pParentWindow->mapToGlobal(pEmbedingWidget->pos() + QPoint(1,1));
				QSize siSize = pEmbedingWidget->size();
				this->setMaximumSize(siSize);
				this->setGeometry(QRect(ptWinPos,siSize));

			}
			else
			{
				TRACE_W(QString("MFormClientDlg::show:Error unable to find visible object named:%1!")
									.arg(m_sEmbedTo));

			}
		}
		
		QWidget::show();
		focusFirstInput();
		setCorrectTabOrdering();
		initMouseCursorInitPos();

		if(m_iInitMousePosX != -1 && m_iInitMousePosY != -1)
		{
			m_iInitMousePosX = m_iInitMousePosX + ptWinPos.x();
			m_iInitMousePosY = m_iInitMousePosY + ptWinPos.y();
		}

		positionMousePointer();


	}
	else
		TRACE_W(QString("MFormClientDlg::show:Error template not loaded!"));
}


bool MFormClientDlg::eventFilter(QObject *obj, QEvent *event)
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

void MFormClientDlg::moveFocusToPreviousInput()
{
	//TO DO - not implemented
}


void MFormClientDlg::moveFocusToNextInput()
{
	QWidget *pInput = focusWidget();
	QWidget *pNext = NULL;

	if(pInput!=NULL)
	{
		pNext = findNextInputToFocus(pInput);

		if(pNext!=NULL)
			pNext->setFocus();
		else
			focusFirstInput();
	}

}


void MFormClientDlg::focusFirstInput()
{
	//set the focus to the first available input control
	QWidget *pNext = findNextInputToFocus(NULL);

	if(pNext!=NULL)
		pNext->setFocus();	
}

QWidget * MFormClientDlg::findNextInputToFocus(QWidget * pCurrentInFocus)
{
	QWidget *pCurrent;
	QWidget *pNext = NULL;
	bool bCurrentFound = false;
	
	if(pCurrentInFocus == NULL)
			bCurrentFound = true; //for null get first that can get focus

	for(int i=0;i<m_lstInputWidgets.size();i++)
	{
		pCurrent = m_lstInputWidgets.at(i);
		
		if(!bCurrentFound && pCurrent==pCurrentInFocus)
		{
			bCurrentFound = true;
			continue;
		}
		else if(bCurrentFound && pCurrent!=NULL && 
				pCurrent->focusPolicy()!=Qt::NoFocus && 
				pCurrent->isEnabled() && 
				pCurrent->isVisible())
		{
			pNext = pCurrent;
			break;
		}
	}

	return pNext;
}



bool MFormClientDlg::keyPressEventHandler(QObject *obj, QKeyEvent * event)
{
	int iKey = event->key();


	if(iKey == Qt::Key_Enter|| iKey == Qt::Key_Return)
	{	
		// if button execute the button
		//else move to the next input 
		if(obj==(QObject *)m_btnOK||obj==(QObject *)m_btnCancel)
		{
			QPushButton *pButton = static_cast<QPushButton *>(obj); 
			if(pButton!=NULL)
						pButton->click();
			return true;
		}	
		else
			moveFocusToNextInput();
			
	}
	else if(iKey == Qt::Key_Space)
	{
		//ignore if button
		if(obj==m_btnOK||obj==m_btnCancel)
			return true;
	}
	else if(iKey == Qt::Key_Escape)
	{
		pressedKeyCancel();
		return true;
	}

	return false;
}



void MFormClientDlg::installInputCtrlEventFilters()
{
	//install the event filters for all input controls
	QWidget *pCurrent;
	
	for(int i=0;i<m_lstInputWidgets.size();i++)
	{
		pCurrent = m_lstInputWidgets.at(i);

		if(pCurrent!=NULL)
		{
			pCurrent->installEventFilter(this);
			pCurrent->installEventFilter(m_pParentWindow);
		}
	}
}

void MFormClientDlg::initMouseCursorInitPos()
{

	QWidget *pCurrent;
	
	for(int i=0;i<m_lstInputWidgets.size();i++)
	{
		pCurrent = m_lstInputWidgets.at(i);

		if(pCurrent!=NULL && ((pCurrent==m_btnOK && m_btnOK->isVisible())|| (pCurrent==m_btnCancel && m_btnCancel->isVisible())))
		{
			QPoint pt = pCurrent->pos();

			m_iInitMousePosX = pt.x() + (m_btnOK->width()/2);
			m_iInitMousePosY = pt.y() + (m_btnOK->height()/2);

				if(pCurrent==m_btnOK)
					break; //if OK found do not continue to search since OK should have priority over Cancel
		}
	}
	
	
}


void MFormClientDlg::pressedKeyValidate()
{
	onButtonOK();
}

void MFormClientDlg::pressedKeyCancel()
{
	onButtonCancel();
}	

void MFormClientDlg::onButtonOK()
{
	if(m_btnOK!=NULL)
		emit accepted();
}

void MFormClientDlg::onButtonCancel()
{
	if(m_btnCancel!=NULL)
		emit rejected();
}

void MFormClientDlg::setCorrectTabOrdering()
{
	QWidget *pFirst;
	QWidget *pNext = NULL;
	
	pFirst = findNextInputToFocus(NULL);

	if(pFirst!=NULL)
		pNext = findNextInputToFocus(pFirst);

	while (pFirst!=NULL && pNext!=NULL)
	{
		QWidget::setTabOrder(pFirst, pNext);
		pFirst = pNext;
		pNext = findNextInputToFocus(pFirst);	
	}
}
