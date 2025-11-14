
#include <QResizeEvent>
#include <QTabBar>
#include "MTracer.h"
#include "MTabView.h"


MTabView::MTabView(QWidget * parent):
	QTabWidget(parent)
{
}

MTabView::~MTabView()
{

}

void MTabView::init(QString sActionId)
{
	m_sActionId	= sActionId;

	m_pTab = new QTabBar(this);
	setTabBar(m_pTab);
	setTabPosition(QTabWidget::North);
	m_pTab->setFocusPolicy(Qt::StrongFocus);
	m_pTab->installEventFilter(this);
}




bool MTabView::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress) 
	{
		 QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		 
		 if(mouseEvent->button()==Qt::LeftButton)
		 {
			 if(obj == m_pTab && m_pTab!=NULL)
			 {
				int iIndex = m_pTab->tabAt(mouseEvent->pos());
				int iCurrentIndex = currentIndex();

				if(iIndex!=-1 && iIndex!=iCurrentIndex)
					emit clicked(m_sActionId, iIndex);

				 return true;
			 }
		 }	
		
	}
	else if(event->type() == QEvent::KeyPress) 
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

		TRACE_D(QString("MTabView::eventFilter: ScanCode:%1 VirtualCode:%2 Key:%3 Text:%4 - Obj:%6")
			.arg(keyEvent->nativeScanCode())
			.arg(keyEvent->nativeVirtualKey())
			.arg(keyEvent->key())
			.arg(keyEvent->text())
			.arg(obj->metaObject()->className()));

			int iKey = keyEvent->key();

		   switch(iKey)
		   {
				case Qt::Key_Left:
				{
					int iCurrentIndex = currentIndex();
					if(iCurrentIndex>0)
					{
						emit clicked(m_sActionId, iCurrentIndex-1);
						return true;
					}
				}
				break;

				case Qt::Key_Right:
				{
					int iCurrentIndex = currentIndex();
				
					if(iCurrentIndex+1<m_pTab->count())
					{
						emit clicked(m_sActionId, iCurrentIndex+1);
						return true;
					}
				}
   				break;

					break;
			   default:
				   break;
		   }


		   return true;

	}
	else if(event->type() == QEvent::Wheel)
	{
		return true;
	}
	
	 // standard event processing
	return QObject::eventFilter(obj, event);

}