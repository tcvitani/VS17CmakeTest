
#include <QVBoxLayout>
#include <QWidget>
#include <QFrame>
#include <QEvent>
#include <QKeyEvent>


#include "MIhmConfigGeneral.h"
#include "MFormInputEx.h"
#include "MInputDialogReq.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}
 

MFormInputEx::MFormInputEx(QWidget *pParentWindow)
    : QWidget(pParentWindow)
{

	setWindowFlags(Qt::Dialog
						| Qt::MSWindowsFixedSizeDialogHint
						| Qt::FramelessWindowHint);

	m_bTemplateLoaded = false;
	m_sResult = "";
	m_pParentWindow = pParentWindow;
	setMouseTracking(true);

	if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
		this->setCursor(Qt::BlankCursor);
	else
		this->setCursor(Qt::ArrowCursor);
}


MFormInputEx::~MFormInputEx()
{

}


void MFormInputEx::show()
{
	QPoint ptWinPos;

	//Show only if template correctly loaded
    if(m_bTemplateLoaded)
	{
		//dialog alignment
		if(!m_oReq.isEmbedded())
		{
			ptWinPos.setX(m_oReq.getPosX());
			ptWinPos.setY(m_oReq.getPosY());

			this->setMaximumSize(m_sTemplateSize);
			this->setGeometry(QRect(ptWinPos,m_sTemplateSize));
		}
		else
		{
			QWidget * pEmbedingWidget = m_pParentWindow->findChild<QWidget *>(m_oReq.getRelativeToObjectName());
			
			if(pEmbedingWidget!=NULL)
			{	
				ptWinPos = m_pParentWindow->mapToGlobal(pEmbedingWidget->pos() + QPoint(1,1));
				QSize siSize = pEmbedingWidget->size();
				this->setMaximumSize(siSize);
				this->setMinimumSize(siSize);
				this->resize(siSize);
				this->move(ptWinPos);
			}
			else
			{
				TRACE_W(QString("MFormInputEx::show:Error unabel to find visible object named:%1!")
									.arg(m_oReq.getRelativeToObjectName()));

			}
		}



		QWidget::show();
		focusFirstInput();
		setCorrectTabOrdering();

		//-----------------
		//Move mouse pointer over the default button
		initMouseCursorInitPos();

		if(m_iInitMousePosX != -1 && m_iInitMousePosY != -1)
		{
			m_iInitMousePosX = m_iInitMousePosX + ptWinPos.x();
			m_iInitMousePosY = m_iInitMousePosY + ptWinPos.y();
		}

		positionMousePointer();
		//-----------------

	}
	else
		TRACE_W(QString("MFormInputEx::show:Error template not loaded!"));
}


bool MFormInputEx::eventFilter(QObject *obj, QEvent *event)
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

void MFormInputEx::closeEvent(QCloseEvent * event)
{
	Q_UNUSED(event)

	emit rejected();
}