#include <QtGui>

#include <QtGui>
#include <QAxWidget>
#include <QWidget>
#include <QUiLoader>
#include "MIhmTaskBarDlg.h"
#include "MKeyMapperWin.h"
#include "MFormMain.h"
#include "MTracer.h"



#define MAX_IE_AX_CONNECTION_RETRYS 100

MFormMain * MIhmTaskBarDlg::m_pMainDlg = NULL;
MMemMSGStruct MIhmTaskBarDlg::m_oOldMSG;	

//---------------------------------------
//	MIhmTaskBarDlg
//---------------------------------------
MIhmTaskBarDlg::MIhmTaskBarDlg(MFormMain *pMainDlg, int nType, QString sTitle)
:QDialog((QWidget *)pMainDlg)
{
	m_iIERetries = 0;
	
	m_iHandleToIE = NULL;
	m_pIEControl = NULL;
	
	m_pMainDlg = pMainDlg;
	m_nType = nType;
	setTitleText(sTitle);

	setWindowFlags(Qt::Window
					| Qt::WindowMinimizeButtonHint 
					| Qt::WindowStaysOnTopHint);

}


MIhmTaskBarDlg::~MIhmTaskBarDlg()
{
	if(m_iHandleToIE!=NULL)	
 		uninstallNewHandler();
	
	if(m_pIEControl!=NULL)	
 		delete	m_pIEControl;
}

bool MIhmTaskBarDlg::initialize(QString sUITemplate, const QRect &rectGeometry)
{
	bool bRetVal = false;
	QUiLoader loader;

	TRACE_D(QString("MIhmTaskBarDlg::initialize: sUITemplate:%1").
						arg(sUITemplate));
	
	QFile file(sUITemplate);
	if(file.open(QFile::ReadOnly))
	{
		m_dynFormWidget = loader.load(&file, this);
		file.close();
		QSize szSize = m_dynFormWidget->size();

		m_layout = new QVBoxLayout(this);
		m_frContainer = this->findChild<QFrame*>("frContainer");
		m_layout->addWidget(m_frContainer);
		
		m_layout->setSpacing(0);
		this->setContentsMargins(0,0,0,0);

		m_frIEContainer = this->findChild<QFrame*>("frIEContainer");
		
		if(m_frIEContainer!=NULL && m_frContainer!=NULL)
		{
			m_pIEControl = new SHDocVw::WebBrowser(this);
			m_pIEControl->installEventFilter(m_pMainDlg);

			QVBoxLayout *frLayout = new QVBoxLayout(m_frIEContainer);
			frLayout->addWidget(m_pIEControl);
			
			frLayout->setSpacing(0);
			frLayout->setContentsMargins(0,0,0,0);
			bRetVal = true;

			
			if(m_pMainDlg!=NULL)
			{
				if(rectGeometry.isValid())
				{
					this->resize(rectGeometry.size());
					this->move(rectGeometry.topLeft());
				}
				else
				{
					this->resize(szSize);
					//make the dialog centered
					int iWidth = m_dynFormWidget->width();
					int iHeight = m_dynFormWidget->height();

					int x = m_pMainDlg->x() + ((m_pMainDlg->width() - iWidth)/2);
					int y = m_pMainDlg->y() + ((m_pMainDlg->height() - iHeight)/2);
 
					this->move(x,y);
				}

			}
		}
	}
	
	if(!bRetVal)
	{
		TRACE_W(QString( "MIhmTaskBarDlg::initialize: Error loading %1").
						arg(sUITemplate));
	}
	else
	{
		//arm the timer to initialize the control
		connect(&tm, SIGNAL(timeout()), this, SLOT(onTimeout()));	
		tm.start(500);
	}

	return bRetVal;
}

void MIhmTaskBarDlg::setTitleText(QString sTitle)
{
	m_sTitle = sTitle;
	setWindowTitle(sTitle);
}

void MIhmTaskBarDlg::loadDefaultPage()
{
	openLink("file://c:/csr/exe/others/emptyWebPage.html");
}


bool MIhmTaskBarDlg::openLink(QString sLink)
{
	if(m_pIEControl!=NULL)
	{
		m_pIEControl->Navigate(sLink);
		return true;
	}
	else 
		return false;
}


struct HwndFuncPair{
	WNDPROC wpOrigEditProc; 
	HWND hwnd;
};

QList <HwndFuncPair *> lstHwndFuncPairs;


HwndFuncPair * findFuncByHwnd(HWND hwnd)
{
	HwndFuncPair * pCurrent;

	for(int i=0; i<lstHwndFuncPairs.size(); i++)
	{
		pCurrent = lstHwndFuncPairs.at(i);
		if(pCurrent->hwnd == hwnd)
		{
			return pCurrent;
		}
	}

	return NULL;
}

// Subclass procedure 
LRESULT APIENTRY EditSubclassProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{ 
	bool bFilter = false;

	MSG msg; 
	msg.hwnd = hwnd;
	msg.message = uMsg;
	msg.wParam = wParam;
	msg.lParam = lParam;

	if(uMsg == WM_KEYDOWN || uMsg == WM_IME_KEYDOWN || uMsg == WM_SYSKEYDOWN || uMsg == WM_IME_CHAR || uMsg == WM_CHAR)
	{
		
		TRACE_D(QString("MIhmTaskBarDlg!! EditSubclassProc: Msg: %1!").arg(uMsg,0,16));
		//Decode exactly the WM_KEYDOWN message to fill correctly the QKeyEvent
		//and post it to the main window
		QKeyEvent * pEvent = MKeyMapperWin::translateToKeyEvent(msg);
		
		if(pEvent!=NULL) 
		{
			char cNewChar;
			QString s = pEvent->text();
			if(s.size()>0) 
				cNewChar = (unsigned char)s.at(0).toLatin1();
			else
				cNewChar = 0;
			
			if(uMsg == WM_KEYDOWN)	 
			{			
				if(MIhmTaskBarDlg::m_pMainDlg->keyPressEventHandler(MIhmTaskBarDlg::m_pMainDlg, pEvent))
					bFilter = true;
			}
			else
			{
				//filter the WM_CHAR messages if it is followed after WM_KEYDOWN which were filtered
				if(MIhmTaskBarDlg::m_oOldMSG.oMsg.message == WM_KEYDOWN && uMsg == WM_CHAR && 
						MIhmTaskBarDlg::m_oOldMSG.bFiltered == true &&
						MIhmTaskBarDlg::m_oOldMSG.cGeneratedChar == cNewChar)
							bFilter = true;
			}

			MIhmTaskBarDlg::m_oOldMSG.oMsg = msg;
			MIhmTaskBarDlg::m_oOldMSG.bFiltered = bFilter;
			MIhmTaskBarDlg::m_oOldMSG.cGeneratedChar = cNewChar;

			delete pEvent;
		}


	}
	

	if(bFilter)
		return 0;

	HwndFuncPair * pPair = findFuncByHwnd(hwnd);
	if(pPair!=NULL)
	{
		WNDPROC wpOrigEditProc = pPair->wpOrigEditProc;

		return CallWindowProc(wpOrigEditProc, hwnd, uMsg, 
			wParam, lParam); 
	}
	else
	{
		TRACE_W(QString("EditSubclassProc: No function for HWND:%1").
						arg((int)hwnd));
	}
	
	return 0;
} 

bool MIhmTaskBarDlg::installNewHandler()
{
	//WNDPROC wpOrigEditProc;

	//wpOrigEditProc = (WNDPROC) SetWindowLongPtr((HWND)m_iHandleToIE, 
 //               GWLP_WNDPROC, (LONG_PTR) EditSubclassProc); 

	//if(wpOrigEditProc!=0)
	//{
	//	HwndFuncPair * pNewPair = new HwndFuncPair();
	//	pNewPair->wpOrigEditProc = wpOrigEditProc;
	//	pNewPair->hwnd = m_iHandleToIE;
	//	
	//	lstHwndFuncPairs.append(pNewPair);
	//}
	//else
	//{
	//	TRACE_W(QString("MIhmTaskBarDlg::installNewHandler: Error SetWindowLongPtr return 0 Error: %1!").
	//					arg(GetLastError()));
	//	
	//	char* lpMsgBuf;
	//	FormatMessageA( 
	//		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
	//		FORMAT_MESSAGE_FROM_SYSTEM | 
	//		FORMAT_MESSAGE_IGNORE_INSERTS,
	//		NULL,
	//		GetLastError(),
	//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	//		(char*) &lpMsgBuf,
	//		0,
	//		NULL 
	//	);
	//	
	//	TRACE_W(QString("%1").arg(lpMsgBuf));

	//	// Free the buffer.
	//	LocalFree( lpMsgBuf );

	//}

	return true;
}

bool MIhmTaskBarDlg::uninstallNewHandler()
{
	//
	//HwndFuncPair * pPair = findFuncByHwnd(m_iHandleToIE);

	//if(pPair!=NULL)
	//{
	//	WNDPROC wpOrigEditProc = pPair->wpOrigEditProc;
	//
	//	SetWindowLongPtr((HWND)m_iHandleToIE, GWLP_WNDPROC, 
	//				(LONG_PTR) wpOrigEditProc); 

	//	delete lstHwndFuncPairs.takeAt(lstHwndFuncPairs.indexOf(pPair));
	//}

	return true;
}
 
 
void MIhmTaskBarDlg::onTimeout()
{
	//if(m_pIEControl->ReadyState()==SHDocVw::READYSTATE_COMPLETE && m_iHandleToIE == NULL)
	//{
	//	HWND iHwndControl = (HWND)m_pIEControl->winId();

	//	HWND iHandle1; HWND iHandle2; HWND iFirstChild;
	//	
	//	iHandle1 = iHwndControl;
	//	
	//	iFirstChild = GetWindow(iHwndControl,  // handle to child window
	//								GW_CHILD);

	//	int iChildNo = 0;

	//	while (m_iHandleToIE==NULL)
	//	{
	//		iHandle2 = GetWindow(iHandle1,  // handle to child window
	//								GW_CHILD);
	//		iChildNo++;

	//		if(iHandle2 != NULL)
	//		{
	//			iHandle1 = iHandle2;
	//		}
	//		else if((iHandle2 == NULL && iHandle1 == iHwndControl)||
	//				(iHandle2 == NULL && iHandle1 == iFirstChild))
	//		{
	//			//if no first child
	//			break; 
	//		}
	//		else
	//		{
	//			m_iHandleToIE = iHandle1;
	//			break;
	//		}
	//	}

	//
	//	if(m_iHandleToIE!=NULL)
	//	{
	//		tm.stop();
	//		installNewHandler();
	//	}

	//}
	//else
	//{
	//	m_iIERetries = m_iIERetries + 1;
	//	
	//	if(m_iIERetries > MAX_IE_AX_CONNECTION_RETRYS)
	//	{
	//		TRACE_W("MIhmTaskBarDlg::onTimeout:Error");
	//		tm.stop();
	//	}
	//}
}




//event handler reimplemented to catch the minimization moment
bool MIhmTaskBarDlg::event(QEvent * event)
{
	if(event->type() == QEvent::WindowStateChange)
	{
		QWindowStateChangeEvent *pEvt = (QWindowStateChangeEvent *)event;
		
		Qt::WindowStates eOldStates = pEvt->oldState();
		Qt::WindowStates eNewStates = windowState();

		bool bWasMinimized = eOldStates & Qt::WindowMinimized;
		bool bIsMinimized = eNewStates & Qt::WindowMinimized;

		if(!bWasMinimized && bIsMinimized)
		{
			//minimized
			emit minimized();
		}
	}

	return QWidget::event(event);
}



