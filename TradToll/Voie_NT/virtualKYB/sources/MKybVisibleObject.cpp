

#include "MKybVisibleObject.h"
#include <QFrame.h>
#include <QBoxLayout>
#include <QHBoxLayout>


extern "C" {
	#include <run.H>
	#include <virt_kyb.H>
};



//--------------------------------------------------------------------


MKybButton::MKybButton(QString sName)
{
	m_sName = sName;
}

MKybButton::~MKybButton()
{

}
		
void MKybButton::initialize(QWidget * pWidget, MKybKeyCfg* pKeyCfg)
{
	m_pWidget = pWidget;

	QObject::connect(pWidget,SIGNAL(clicked()),this, SLOT(onTriggered()));
	
	m_KeyCfg = *pKeyCfg;
}


void MKybButton::onTriggered()
{
	KybEvent * pCurrent;

	for(int i=0; i<m_KeyCfg.m_lstKybEvents.count(); i++)
	{
		pCurrent = m_KeyCfg.m_lstKybEvents.at(i);

		keybd_event(pCurrent->bVk, 0, pCurrent->dwFlags, 0);

	}

}