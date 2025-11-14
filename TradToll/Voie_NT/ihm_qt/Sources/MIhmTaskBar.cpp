#include <QtGui>

#include "mihmtaskbar.h"
#include "MIhmTaskBarDlg.h"
#include "MIhmTaskBarButton.h"
#include "MTracer.h"


//---------------------------------------
//	MIhmTaskBar
//---------------------------------------
MIhmTaskBar::MIhmTaskBar(QFrame *parent)
:QFrame(parent)
{
	m_pParentWindow = NULL;
	//make horizontal layout
	m_horizontalLayout = new QHBoxLayout(parent);
	m_horizontalLayout->setSpacing(5);
	m_horizontalLayout->setContentsMargins(1,1,1,1);
	m_horizontalLayout->setDirection(QBoxLayout::LeftToRight);
	m_horizontalLayout->addStretch(0);

}

MIhmTaskBar::~MIhmTaskBar()
{
	while(!m_lstDlgButtons.isEmpty())
		delete m_lstDlgButtons.takeFirst();
}

void MIhmTaskBar::init(MFormMain *pMainWindow, QString sDlgUITemplatePath)
{
	m_pParentWindow = pMainWindow;
	m_sDlgUITemplatePath = sDlgUITemplatePath;
}

void MIhmTaskBar::initDialog(int nType, QString sName,  const QRect &rectGeometry )
{
	addNewDialog(nType, sName, rectGeometry);
}

void MIhmTaskBar::addNewDialog(int nType, QString sName, const QRect &rectGeometry)
{
	////create browser dialog
	//MIhmTaskBarDlg *dlg;
	//dlg = new MIhmTaskBarDlg(m_pParentWindow, nType, sName);
	//dlg->initialize(m_sDlgUITemplatePath, rectGeometry);
	//dlg->loadDefaultPage();

	//MIhmTaskBarButton *pNewButton = new MIhmTaskBarButton(this, sName);
	//QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//pNewButton->setSizePolicy(sp);

	//pNewButton->initialize(dlg);
	//pNewButton->hide();

	////add button to list of buttons
	//m_lstDlgButtons.append(pNewButton);
	//m_horizontalLayout->addWidget(pNewButton);
	
}

MIhmTaskBarButton * MIhmTaskBar::findButton(int nType)
{
	MIhmTaskBarButton * pRetVal = NULL;

	MIhmTaskBarButton * pCurrent;
	
	for(int i=0; i<m_lstDlgButtons.size();i++) 
	{
		pCurrent = m_lstDlgButtons.at(i);

		if(pCurrent->getType() == nType)
		{
			pRetVal = pCurrent;
			break;
		}
	}
	
	return pRetVal;
}


void MIhmTaskBar::openBrowserDlg(int nType, QString sLink)
{
	MIhmTaskBarButton * pButton = NULL;
	pButton = findButton(nType);

	if(pButton != NULL)
	{  
		pButton->openLink(sLink);
		
		int iWidth = parentWidget()->width();
		iWidth = (iWidth-15)/3;
		pButton->setMinimumWidth(iWidth);
	}
}


void MIhmTaskBar::closeBrowserDlg(int nType)
{
	MIhmTaskBarButton * pButton = NULL;
	pButton = findButton(nType);

	if (pButton != NULL)
	{
		pButton->makeClosed();
	}
}

void MIhmTaskBar::closeAll()
{
	MIhmTaskBarButton * pCurrent;
	
	for(int i=0; i<m_lstDlgButtons.size();i++) 
	{
		pCurrent = m_lstDlgButtons.at(i);
		pCurrent->makeClosed();
	}
}


void MIhmTaskBar::minimizeAll()
{
	MIhmTaskBarButton * pCurrent;
	
	for(int i=0; i<m_lstDlgButtons.size();i++) 
	{
		pCurrent = m_lstDlgButtons.at(i);

		pCurrent->makeMinimized();
	}

}

