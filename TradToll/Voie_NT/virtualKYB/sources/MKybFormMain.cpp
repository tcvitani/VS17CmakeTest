#include <windows.h>

#include <QtCore>
#include <QtGui>
#include <QHBoxLayout>
#include <QUiLoader>
#include <QFont>
#include <QFrame>
#include <QFileInfo>
#include <QPushButton>

#include "MTracer.h"
#include "MKybFormMain.h"
#include "MKybVisibleObject.h"
#include "MKybConfigGeneral.h"


MKybFormMain::MKybFormMain(bool bShowTitle, QWidget *parent)
    : QMainWindow(parent)
{
	m_bTemplateLoaded = false;

	if(!bShowTitle) 
		setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ); 
	else
		setWindowFlags(Qt::WindowStaysOnTopHint); 

	setFocusPolicy(Qt::NoFocus);
	SetWindowLong ((HWND)this->winId(), GWL_EXSTYLE, GetWindowLong((HWND)this->winId(), GWL_EXSTYLE) | WS_EX_NOACTIVATE);

	if (MKybConfigGeneral::getCfg()->shouldHideCursor())
		this->setCursor(Qt::BlankCursor);
	else
		this->setCursor(Qt::ArrowCursor);
}

MKybFormMain::~MKybFormMain()
{

// 	if(m_layout!=NULL)
// 		delete m_layout;
	
	while (!m_lstCurrentButtons.isEmpty())
		delete m_lstCurrentButtons.takeFirst();

}

void MKybFormMain::center()
{
	//make the window appear at the center
	QScreen* myScr = qApp->primaryScreen();
	QRect screenRect = myScr->availableGeometry();
	QRect rectTarget = this->geometry();
	
	
	int x = (screenRect.width() - rectTarget.width())/2;
	int y = (screenRect.height() - rectTarget.height())/2; 
	
	x = (x>0)?x:0;
	y = (y>0)?y:0;

	TRACE_D(QString("MKybFormMain::center: Screen:%1 x %2, form:%3 x %4, position:X:%5 Y:%6")
									.arg(screenRect.width())
									.arg(screenRect.height())
									.arg(rectTarget.width())
									.arg(rectTarget.height())
									.arg(x)
									.arg(y));

	this->move(x,y);
}

void MKybFormMain::showMainDlg()
{
	//Show only if template correctly loaded
    if(m_bTemplateLoaded)
		show();
}


bool MKybFormMain::initialize(MKybTypeSettings * pNewSettings)
{
	bool bRetVal = false;

 	TRACE_D(QString("MKybFormMain::initialize: Loading keyboard ID: %1").arg(pNewSettings->getCfgId()));

	QString sDialogUIPath = pNewSettings->getUIFileNamePath();
	m_sCfgKyboardFileName = pNewSettings->getCfgFileName();

	bRetVal = loadTemplate(sDialogUIPath);
	
	if(bRetVal)
	{
		if(connectVisibleObjects(pNewSettings->getKybKeysList()))
		{	
			m_bTemplateLoaded = true;

			bRetVal = true;
		}
		else
		{
			TRACE_W(QString("MKybFormMain::loadTemplate: Error connectVisibleObjects %1")
									.arg(sDialogUIPath));
			bRetVal = false;
		}

	}
	else
	{
		TRACE_W(QString( "MKybFormMain::initialize: Error loading template: %1 parameter for the %2 keyboard ID!")
									.arg(sDialogUIPath)
									.arg(pNewSettings->getCfgId()));
		bRetVal = false;
	}


	return bRetVal;
}





bool MKybFormMain::loadTemplate(QString sUIFilePath)
{
	bool bRetVal = false;
	QUiLoader loader;

	TRACE_D(QString("MKybFormMain::loadTemplate: UI from file %1")
									.arg(sUIFilePath));

	QFile file(sUIFilePath);
	
	if(file.open(QFile::ReadOnly))
	{
		QHBoxLayout *m_layout = new QHBoxLayout(this);
		QFrame *m_mainFrame = new QFrame(this);
		m_mainFrame->setFrameShape(QFrame::NoFrame);
		m_mainFrame->setLineWidth(0);
		
		//NOTE m_dynFormWidget is actually not a real widget and it cannot be used to add to layouts or set the styles
		m_dynFormWidget = loader.load(&file, m_mainFrame);
		file.close();

		QSize size = m_dynFormWidget->size();
		
		//keep the original style sheet to be able to dynamically add style
		m_sDynaFormStyleSheet = m_dynFormWidget->styleSheet();	
		
		m_layout->addWidget(m_mainFrame);
		m_layout->setSpacing(0);
		m_layout->setContentsMargins(1,1,1,1);
		
		this->setContentsMargins(0,0,0,0);
		this->setCentralWidget(m_mainFrame);

		m_frContainer = m_mainFrame->findChild<QFrame*>("frMainFrame");
		
		if(m_frContainer != NULL)
		{
			QLayout * pLayout = new QHBoxLayout(m_mainFrame);
			pLayout->addWidget(m_frContainer);
 
 			pLayout->setContentsMargins(0,0,0,0);
			this->resize(size);
			m_mainFrame->setStyleSheet(m_sDynaFormStyleSheet);
			setWindowTitle(QString("Virtual keyboard: %1").arg(m_sCfgKyboardFileName));
			bRetVal = true;
		}
	}
	else
	{
		TRACE_W(QString("MKybFormMain::loadTemplate: Error loading %1")
									.arg(sUIFilePath));
		return false;
		
	}

	return bRetVal;
}




bool MKybFormMain::connectVisibleObjects(QList <MKybKeyCfg*> * pLst)
{
	bool bRetVal = true;
	QPushButton * pPushButton;
	MKybKeyCfg* pCurrentKeyCfg;

	for(int i=0;i<pLst->size();i++)
	{
		pCurrentKeyCfg = pLst->at(i);

		if (pCurrentKeyCfg == NULL)
		{
			TRACE_W(QString( "MKybFormMain::connectVisibleObjects: Error in cfg list item: %1")
									.arg(i));
			continue;
		}

		pPushButton = this->findChild<QPushButton*>(pCurrentKeyCfg->getWidgetName());
		
		if(pPushButton!=NULL)
		{
			MKybButton * pNewButton = new MKybButton(pCurrentKeyCfg->getWidgetName());
 			pNewButton->initialize(pPushButton, pCurrentKeyCfg);
			m_lstCurrentButtons.append(pNewButton);
			pPushButton->setFocusPolicy(Qt::NoFocus);
		}
		else
		{
			TRACE_W(QString( "MKybFormMain::connectVisibleObjects: Unable to find template object %1")
									.arg(pCurrentKeyCfg->getWidgetName()));
			bRetVal = false;
		}
	}

	return bRetVal;
}



void MKybFormMain::closeEvent(QCloseEvent * event)
{
	event->ignore();
	emit stopRequested();
}


