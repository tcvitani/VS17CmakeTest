
#include <QKeyEvent>
#include <QAbstractItemView>

#include "MIhmComboBox.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <reg.h>
	#include "ihm.h"
}

MIhmComboBox::MIhmComboBox(QWidget *parent)
    : QComboBox(parent)
{
	m_bAutoOpen = true;
}

MIhmComboBox::~MIhmComboBox()
{
}

void MIhmComboBox::focusInEvent(QFocusEvent * event)
{
	if(this->isVisible()&&!this->view()->isVisible() && m_bAutoOpen)
	{
		showPopup();
	}
	
	if(!this->view()->isVisible())
					emit gotFocus();

	QComboBox::focusInEvent(event);
}

void MIhmComboBox::keyPressEvent(QKeyEvent * event)
{
	int iKey = event->key();

	if(this->isVisible() && iKey == Qt::Key_Up && !this->view()->isVisible())
	{
		showPopup();
	}
	else if(this->isVisible() && iKey == Qt::Key_Down && !this->view()->isVisible())
	{
		showPopup();
	}
	else
		QComboBox::keyPressEvent(event);
}

