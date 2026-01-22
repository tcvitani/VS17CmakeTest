#include <QScreen>

#include <QFileInfo>
#include <QMouseEvent>

#include "MStyledTitleBar.h"



MStyledTitleBar::MStyledTitleBar(QWidget *parentDialog, QWidget *parentFrame, QAbstractButton *pbtnMinimize, QAbstractButton *pbtnMaximize, QAbstractButton *pbtnClose)
{
	m_parentDialog = parentDialog;
	m_parentFrame = parentFrame;

 	m_parentDialog->installEventFilter(this);
 	m_parentDialog->setMouseTracking(true);

	QList<QWidget *> widgets = m_parentDialog->findChildren<QWidget *>();
	foreach(QWidget * p, widgets)
	{
		p->setMouseTracking(true);
		p->installEventFilter(this);
	}


    // Don't let this widget inherit the parent's backround color
    setAutoFillBackground(true);

    // Use a brush with a Highlight color role to render the background 
    setBackgroundRole(QPalette::Highlight);

	m_pbtnMinimize = pbtnMinimize;
	m_pbtnMaximize = pbtnMaximize;
	m_pbtnClose = pbtnClose;
	
	if(m_pbtnMaximize!=NULL)
	{
		connect(m_pbtnMaximize, SIGNAL( clicked() ), this, SLOT(showMaxRestore() ) );
	}

 	if(m_pbtnMinimize!=NULL)
	{
		connect(m_pbtnMinimize, SIGNAL( clicked() ), this, SLOT(showSmall() ) );
	}

 	if(m_pbtnClose!=NULL)
	{
		connect(m_pbtnClose, SIGNAL( clicked() ), parentDialog, SLOT(close() ) );
	}

    bIsMaximized = false;
   

	m_mouse_down = false;
	m_bResizeStarted = false;
	m_bIsMouseDownAtTitle = false;
}

void MStyledTitleBar::setButtonsPixmaps(const QPixmap &pxMaximize, const QPixmap &pxMinimize, const QPixmap &pxRestore, const QPixmap &pxClose)
{
	m_pxMaximize = pxMaximize;
	m_pxMinimize = pxMinimize;
	m_pxRestore = pxRestore;
	m_pxClose = pxClose;
	
	if(!m_pxMaximize.isNull() && m_pbtnMaximize!=NULL)
	{	m_pbtnMaximize->setIcon(m_pxMaximize);
        m_pbtnMaximize->setIconSize(QSize(48, 48));
	}

	if(!m_pxMinimize.isNull()  && m_pbtnMinimize!=NULL)
	{	m_pbtnMinimize->setIcon(m_pxMinimize);
        m_pbtnMaximize->setIconSize(QSize(48, 48));
	}

	if(!m_pxClose.isNull() && m_pbtnClose!=NULL)
	{	m_pbtnClose->setIcon(m_pxClose);
        m_pbtnMaximize->setIconSize(QSize(48, 48));
	}


}

void MStyledTitleBar::showSmall()
{
     m_parentDialog->showMinimized();
}

void MStyledTitleBar::showMaxRestore()
{
    if (bIsMaximized) {

        m_parentDialog->showNormal();
		m_parentDialog->resize(m_normalSize);
		m_parentDialog->move(m_normalPos);

        bIsMaximized = !bIsMaximized;
 		
		if(!m_pxMaximize.isNull() && m_pbtnMaximize!=NULL)
			{	m_pbtnMaximize->setIcon(m_pxMaximize);
				m_pbtnMaximize->setIconSize(QSize(48, 48));
			}
    } else {
		m_normalPos = m_parentDialog->pos();
		m_normalSize = m_parentDialog->size();

        m_parentDialog->showMaximized();
		QScreen * p = this->screen();
		m_parentDialog->resize(p->availableGeometry().width(), p->availableGeometry().height());
		m_parentDialog->move(0,0);

        bIsMaximized = !bIsMaximized;

		if(!m_pxRestore.isNull() && m_pbtnMaximize!=NULL)
			{	m_pbtnMaximize->setIcon(m_pxRestore);
				m_pbtnMaximize->setIconSize(QSize(48, 48));
			}
    }
}



bool MStyledTitleBar::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::MouseMove ) 
	{
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		QWidget * w = static_cast<QWidget *>(obj);

		mouseMoveEvent(w, mouseEvent);
	}
	if(event->type() == QEvent::MouseButtonPress) 
	{
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		QWidget * w = static_cast<QWidget *>(obj);
		mousePressEvent(w, mouseEvent);
	}
	else if(event->type() == QEvent::MouseButtonRelease) 
	{
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		QWidget * w = static_cast<QWidget *>(obj);
		mouseReleaseEvent(w, mouseEvent);
	}
	else if(event->type() == QEvent::Resize) 
	{
		if(obj==m_parentDialog)
		{
// 			QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
// 			mouseResizeEvent(resizeEvent);
			emit dialogResized();
		}
	} 

	// standard event processing
	 return QObject::eventFilter(obj, event);
}

void MStyledTitleBar::mousePressEvent(QWidget * w, QMouseEvent *e)

{
    m_old_pos = e->globalPos();
    m_start_pos = m_parentDialog->pos();
	QPoint ptCurrentPos = e->globalPos();
    int x = ptCurrentPos.x();
    int y = ptCurrentPos.y();
    QRect r = m_parentDialog->geometry();

	if(w == m_parentDialog || w == m_parentFrame)
		m_mouse_down = (e->button() == Qt::LeftButton)?true:false;

	if(w == m_parentFrame)
	{
		QRect g = m_parentFrame->geometry();
		
		QPoint click_pos = e->pos();
		m_bIsMouseDownAtTitle = g.contains(click_pos); 
	}

	if(!m_bResizeStarted && m_mouse_down)
	{
		left = qAbs(x - r.left()) <= 5;
		right = qAbs(x - r.right()) <= 5;
		bottom = qAbs(y - r.bottom()) <= 5;

	 	displayMessage(QString("mouseMoveEvent: left:%1,right:%2,bottom:%3").arg(left).arg(right).arg(bottom));
        
		if(!m_bResizeStarted && (left | right | bottom))
			m_bResizeStarted = true;
    }


	displayMessage(QString("mousePressEvent:widget:%6 m_start_pos x:%1,y:%2, Oldpos:x:%3,y:%4 - MouseDown:%5")
		.arg(m_start_pos.x()).arg(m_start_pos.y()).arg(m_old_pos.x()).arg(m_old_pos.y()).arg(m_mouse_down)
		.arg(w->objectName()));
	
}

   

void MStyledTitleBar::mouseMoveEvent(QWidget * w, QMouseEvent *e)

{
	QPoint ptNewPos = e->globalPos();

	displayMessage(QString("mouseMoveEvent:widget:%6 x:%1,y:%2, Oldpos:x:%3,y:%4 - MouseDown:%5")
		.arg(ptNewPos.x()).arg(ptNewPos.y()).arg(m_old_pos.x()).arg(m_old_pos.y()).arg(m_mouse_down).arg(w->objectName()));

	QPoint ptCurrentPos = e->globalPos();
    int x = ptCurrentPos.x();
    int y = ptCurrentPos.y();
    QRect r = m_parentDialog->geometry();

	if(!m_bResizeStarted)
	{
		left = qAbs(x - r.left()) <= 5;
		right = qAbs(x - r.right()) <= 5;
		bottom = qAbs(y - r.bottom()) <= 5;

	 	displayMessage(QString("mouseMoveEvent: left:%1,right:%2,bottom:%3").arg(left).arg(right).arg(bottom));
    
		bool hor = left | right;
    
		if (hor && bottom && !bIsMaximized)
		{
			if (left)
				m_parentDialog->setCursor(Qt::SizeBDiagCursor);
			else 
				m_parentDialog->setCursor(Qt::SizeFDiagCursor);
		} 
		else if (hor && !bIsMaximized) 
		{
			m_parentDialog->setCursor(Qt::SizeHorCursor);
		} 
		else if (bottom && !bIsMaximized) 
		{
			m_parentDialog->setCursor(Qt::SizeVerCursor);
		} 
		else 
			m_parentDialog->setCursor(Qt::ArrowCursor);
    }

    
	if (m_mouse_down && !bIsMaximized) {
		
		QRect g = m_parentDialog->geometry();

		displayMessage(QString("Geometry: x:%1,y:%2, width:%3 height:%4").arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height()));


		if(m_bResizeStarted)
		{
			//resize
            int dx = ptCurrentPos.x() - m_old_pos.x();
            int dy = ptCurrentPos.y() - m_old_pos.y();

			displayMessage(QString("DIF: dx:%1,dy:%2").arg(dx).arg(dy));
			
            if (left)
                g.setLeft(g.left() + dx);
            if (right)
                g.setRight(g.right() + dx);
            
			if (bottom )
                g.setBottom(g.bottom() + dy);
            
            m_parentDialog->setGeometry(g);

			m_old_pos = ptCurrentPos;
		}
		else
		{
			if(m_bIsMouseDownAtTitle)
				m_parentDialog->move(ptCurrentPos - m_old_pos + m_start_pos);
        }

 		g = geometry();
		displayMessage(QString("Geometry modified: x:%1,y:%2, width:%3 height:%4").arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height()));
    } 

}


void MStyledTitleBar::mouseReleaseEvent(QWidget * w, QMouseEvent *e)
{
    m_mouse_down = false;
	m_bResizeStarted = false;
	m_parentDialog->repaint();
	m_bIsMouseDownAtTitle = false;
}


void MStyledTitleBar::displayMessage(QString sMsg)
{
	//Uncomment if debug trace is needed
	//emit message(sMsg);
}
