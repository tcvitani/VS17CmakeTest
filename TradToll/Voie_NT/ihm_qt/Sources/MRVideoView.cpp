#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QSettings>

#include "MRVideoView.h"
#include "MTracer.h"

MRVideoView::MRVideoView(QWidget *parent)
    : QFrame(parent)
{
	m_bInitialized = false;
	m_parent = parent;
	m_CamCtl = NULL;
}

MRVideoView::~MRVideoView()
{
	if(m_CamCtl!=NULL)
	{
		m_CamCtl->hide();
		m_CamCtl->Stop();
		QObject::disconnect(m_CamCtl, SIGNAL(exception(int,QString,QString,QString)), this, SLOT(onCtlException(int,QString,QString,QString)));
		delete m_CamCtl;
		m_CamCtl = NULL;
	}
}

void MRVideoView::onCtlException(int iCode,QString s1,QString s2,QString s3)
{
	TRACE_W(QString("MRVideoView::onCtlException: ERROR CODE:%1, %2 %3 %4").arg(iCode,0,16).arg(s1).arg(s2).arg(s3));

}


bool MRVideoView::reInitAxis()
{
	if(m_CamCtl!=NULL)
	{
		m_CamCtl->hide();
		m_CamCtl->Stop();
		QObject::disconnect(m_CamCtl, SIGNAL(exception(int,QString,QString,QString)), this, SLOT(onCtlException(int,QString,QString,QString)));
		delete m_CamCtl;
		m_CamCtl = NULL;
	}

	m_CamCtl = new CS_MJPG_CONTROLLib::CS_MjpgCtrl(this);
    
	if(m_CamCtl!=NULL && !m_CamCtl->isNull())
	{
		m_layout->addWidget(m_CamCtl);
		
		QObject::connect(m_CamCtl, SIGNAL(exception(int,QString,QString,QString)), this, SLOT(onCtlException(int,QString,QString,QString)));

// 		m_CamCtl->SetMediaType("mjpeg-unicast");
// 		m_CamCtl->SetShowStatusBar(false);
// 		m_CamCtl->SetShowToolbar(false);
		m_CamCtl->SetStretchToFit(true);
// 		m_CamCtl->SetDisplayMessages(false);
		m_bInitialized = true;
	}
	else
		m_bInitialized = false;


	return m_bInitialized;
}


bool MRVideoView::init()
{
	m_layout = new QHBoxLayout(this);
	m_layout->setSpacing(0);
	m_layout->setContentsMargins(1,1,1,1);
	
	return reInitAxis();
}


bool MRVideoView::SetSource(QString sURL, QString sUser, QString sPwd)
{
	bool bRet = true;

	TRACE_D(QString("MRVideoView::SetSource(): source:[%1]").arg(sURL));

	
	m_sURL = sURL;

	if(m_bInitialized)
	{
		m_CamCtl->hide();
		TRACE_D(QString("MRVideoView::SetSource(): m_CamCtl hidden"));

		m_CamCtl->Stop();
		TRACE_D(QString("MRVideoView::SetSource(): m_CamCtl Stopped"));

// 		m_CamCtl->SetMediaUsername(sUser);
// 		m_CamCtl->SetMediaPassword(sPwd);
		m_CamCtl->SetMediaURL(sURL);
		TRACE_D(QString("MRVideoView::SetSource(): m_CamCtl SetMediaURL OK"));

//		m_CamCtl->SetEnableReconnect(false);
	}
	else
		TRACE_D(QString("MRVideoView::SetSource(): m_CamCtl Not initialized yet!"));


	return bRet;
}


bool MRVideoView::ShowVideo()
{
	if(m_bInitialized)
	{
// 		m_CamCtl->SetEnableReconnect(true);
		m_CamCtl->Stop();
		m_CamCtl->SetMediaURL(m_sURL);
		m_CamCtl->Play();
		m_CamCtl->show();
	}

	return true;
}

bool MRVideoView::HideVideo()
{
	if(m_bInitialized)
	{
		m_CamCtl->hide();
// 		m_CamCtl->SetEnableReconnect(false);
		m_CamCtl->Stop();
		//m_CamCtl->SetMediaURL(""); //to force disconnecting from the streaming server (not sure if necessary since the video is stopped)
	}

	return true;
}



