#include <QtGui>
#include <QMenu>
#include "MIhmTaskBarButton.h"
//#include "MIhmTaskBarDlg.h"


//---------------------------------------
//	MIhmTaskBarButton
//---------------------------------------
MIhmTaskBarButton::MIhmTaskBarButton(QWidget *parent, QString sName):
QPushButton(parent)
{
	connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));
	setText(sName);
}

MIhmTaskBarButton::~MIhmTaskBarButton()
{
	if(m_dlg!=NULL)
		delete m_dlg;
}


//void MIhmTaskBarButton::initialize(MIhmTaskBarDlg *dlg)
//{
//	if(dlg!=NULL)
//	{
//		m_dlg = dlg;
//
//		connect(m_dlg, SIGNAL(rejected()), this, SLOT(onDlgClosed()));
//
//		//must be queued connection otherwise the hide does not work
//		connect(m_dlg, SIGNAL(minimized()), this, SLOT(onDlgMinimize()), Qt::QueuedConnection);
//	}
//	
//}

void MIhmTaskBarButton::onClicked()
{
	makeDialogVisible();
}

void MIhmTaskBarButton::onDlgClosed()
{
	makeClosed();
}

void MIhmTaskBarButton::onDlgMinimize()
{
	makeMinimized();
}



void MIhmTaskBarButton::makeMinimized()
{
	if(m_dlg!=NULL && !m_bClosed)
	{
		this->show();
		m_dlg->hide();
	}
}

void MIhmTaskBarButton::makeDialogVisible()
{
	if(m_dlg!=NULL && !m_bClosed)
	{
		this->hide();
		m_dlg->showNormal();
	}
}

void MIhmTaskBarButton::makeClosed()
{
	m_bClosed = true;
	
	m_dlg->hide();
	this->hide();

}



bool MIhmTaskBarButton::openLink(QString sLink)
{
	if(m_dlg!=NULL)
	{
		m_bClosed = false;

		if(m_dlg->openLink(sLink))
		{
			makeDialogVisible();
			return true;
		}

	}

	return false;
}
















