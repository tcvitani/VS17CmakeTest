#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QSettings>

#include "MRVideoView.h"
#include "MTracer.h"

#include "MJPEGStreamer.h"

MRVideoView::MRVideoView(QWidget *parent)
    : QFrame(parent)
{
	m_bInitialized = false;
	m_parent = parent;
	m_mjpgStreamaer = new MJPEGStreamer(this);

}

MRVideoView::~MRVideoView()
{
	delete m_mjpgStreamaer;

}

bool MRVideoView::reInitAxis()
{
	m_layout->addWidget(m_mjpgStreamaer);
	m_mjpgStreamaer->hide();
	m_bInitialized = true;

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
		m_mjpgStreamaer->hide();
		TRACE_D(QString("MRVideoView::SetSource(): m_CamCtl hidden"));

		m_mjpgStreamaer->stop();
		TRACE_D(QString("MRVideoView::SetSource(): m_CamCtl Stopped"));
	}
	else
		TRACE_D(QString("MRVideoView::SetSource(): m_CamCtl Not initialized yet!"));


	return bRet;
}


bool MRVideoView::ShowVideo()
{
	if(m_bInitialized)
	{
		m_mjpgStreamaer->set_url(m_sURL);
		m_mjpgStreamaer->start();
		m_mjpgStreamaer->show();
	}

	return true;
}

bool MRVideoView::HideVideo()
{
	if(m_bInitialized)
	{
		m_mjpgStreamaer->stop();
		m_mjpgStreamaer->hide();
	}

	return true;
}



