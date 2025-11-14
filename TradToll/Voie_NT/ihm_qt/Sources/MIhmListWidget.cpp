#include "MIhmListWidget.h"

#include <QKeyEvent>

MIhmListWidget::MIhmListWidget(QWidget *parent)
:QListWidget(parent)
{
	
}

MIhmListWidget::~MIhmListWidget()
{

}

void MIhmListWidget::keyPressEvent(QKeyEvent *event)
{
	int iKey = event->key();
	int iModifier = event->modifiers();

	QListWidget::keyPressEvent(event);
	
	int iCurrentRow = currentRow();
	
	if(iCurrentRow>=0)
	{
		//deal with list keyboard input
		if(iKey == Qt::Key_Up)
		{
			if(iModifier != Qt::ControlModifier)
			{
				clearSelection();
				item(iCurrentRow)->setSelected(true);
			}
		}
		else if(iKey == Qt::Key_Down)
		{
			if(iModifier != Qt::ControlModifier)
			{
				clearSelection();
				item(iCurrentRow)->setSelected(true);
			}
		}
	}

}
