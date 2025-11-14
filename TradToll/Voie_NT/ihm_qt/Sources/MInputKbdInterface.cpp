

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QCursor>

#include "MInputKbdInterface.h"


 

MInputKbdInterface::MInputKbdInterface()
{
	m_iInitMousePosX = -1;
	m_iInitMousePosY = -1;

}


MInputKbdInterface::~MInputKbdInterface()
{

}


void MInputKbdInterface::positionMousePointer()
{
	if(m_iInitMousePosX != -1 && m_iInitMousePosY != -1)
	{
		QCursor::setPos(m_iInitMousePosX, m_iInitMousePosY);
	}

}






