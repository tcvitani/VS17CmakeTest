
#include <QVBoxLayout>
#include "MStyledToolBox.h"



MStyledToolBox::MStyledToolBox(QWidget *parentDialog)
	:QToolBox(parentDialog), m_bAutoHide(false), m_uiAutoHideTimeout(300), m_bMouseIn(false)
{
	m_parentDialog = parentDialog;
	
    QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Expanding);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(sizePolicy().hasHeightForWidth());
    setSizePolicy(sizePolicy2);
	
	iCurrentGroup = -1;
	
	connect(this,SIGNAL(currentChanged(int)), this, SLOT(onCurrentGroupChanged(int)));
	connect(&m_tmMouseOut,SIGNAL(timeout()), this, SLOT(onMouseLeaveTimeout()));
	m_tmMouseOut.setSingleShot(true);
	
}

MStyledToolBox::~MStyledToolBox()
{

}

void MStyledToolBox::setFixedWidth(int iWidth)
{
    setMinimumSize(QSize(iWidth, 0));
    setMaximumSize(QSize(iWidth, 16777215));


}

void MStyledToolBox::show()
{
	QToolBox::show();
	m_tmMouseOut.start(2000);

}

void MStyledToolBox::hide()
{
	QToolBox::hide();
	m_tmMouseOut.stop();
}

void MStyledToolBox::leaveEvent( QEvent * event )
{
	m_tmMouseOut.start(m_uiAutoHideTimeout);
	m_bMouseIn = false;

	QToolBox::leaveEvent(event);
}

void MStyledToolBox::enterEvent(QEnterEvent* event )
{
	m_tmMouseOut.stop();
	m_bMouseIn = true;

	QToolBox::enterEvent(event);
}

void MStyledToolBox::onMouseLeaveTimeout()
{
	if(m_bAutoHide && !m_bMouseIn)
		hide();
}

//TO DO ...
// - connect signals to receive signals when clicked for every toolbar buttons
// - simulate all buttons unselected state (main menu options toggle)
int MStyledToolBox::addGroup(const QString & text )
{
    QWidget * pPage = new QWidget();
	
    QVBoxLayout *verticalLayout_4 = new QVBoxLayout(pPage);
    verticalLayout_4->setSpacing(0);
    //verticalLayout_4->setMargin(0);
    verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_4->addItem(verticalSpacer);
	
    int iActualIndex = addItem(pPage, text);
	sGroupLabels.append(text);

	updateToolboxButtonList();

	return iActualIndex;
}


int MStyledToolBox::addGroupItem( int indexGroup, const QString & text, QPushButton **ppButton )
{
    QWidget * pPage = widget(indexGroup);
	if(pPage!=NULL)
	{
		QVBoxLayout *verticalLayout = (QVBoxLayout *)pPage->layout();
        
		if(verticalLayout!=NULL)
		{
			QPushButton *pushButton = new QPushButton(pPage);
			pushButton->setObjectName(text); //set text label to object name property to be able to retranslate to other language
			pushButton->setText(text);
			pushButton->setCursor(QCursor(Qt::PointingHandCursor));
			verticalLayout->insertWidget(verticalLayout->count()-1, pushButton);
			
			if(ppButton!=NULL)
				*ppButton = pushButton;

			return verticalLayout->indexOf(pushButton);
		}
	}
	
	return -1;
}


void MStyledToolBox::onToolboxButtonClicked()
{
	QAbstractButton *pSender = qobject_cast<QAbstractButton *>(sender());

	for(int i=0; i<m_lstToolboxButtons.size();  i++)	
	{
		QAbstractButton * p = m_lstToolboxButtons.at(i);
		
		if(p==pSender)
		{
			groupClicked(i);		
			break;
		}

	}	
}

void MStyledToolBox::onCurrentGroupChanged(int index)
{
	//groupClicked(index);
}


void MStyledToolBox::groupClicked(int index)
{
	if(iCurrentGroup == index && currentIndex() == index)
	{
		iCurrentGroup = -1;
	}
	else
	{
		iCurrentGroup = currentIndex();
	}

	updateToolboxPages();
}

void MStyledToolBox::updateToolboxPages()
{
	if(iCurrentGroup == -1)
	{
		setCurrentIndex(count()-1);	

		widget(count()-1)->hide();
	}
	else
	{
		widget(count()-1)->show();
	}
}

void MStyledToolBox::collapseToolboxMenu()
{
	iCurrentGroup = -1;
	updateToolboxPages();

}


void MStyledToolBox::updateToolboxButtonList()
{
	//get pointers to all toolbox buttons and set minimum height
	QList<QAbstractButton *> lstButtons =  this->findChildren<QAbstractButton *>(QLatin1String("qt_toolbox_toolboxbutton"));
	
	QAbstractButton * pCurrent = NULL;
	int i,j,iFoundIndex;
	bool bFound = false,bInvalidOrder = false;

	for(i=0; i<lstButtons.size();  i++)	
	{
		QAbstractButton * p = lstButtons.at(i);
		p->setMinimumHeight(90);
		bFound = false;

		for(j=0; j<m_lstToolboxButtons.size();  j++)	
		{
			pCurrent = m_lstToolboxButtons.at(j);
			
			if(p==pCurrent)
			{
				bFound = true;
				iFoundIndex = j;
			}
		}
		
		if(!bFound)
		{
			m_lstToolboxButtons.append(p);
			p->setCursor(QCursor(Qt::PointingHandCursor));
			connect(p,SIGNAL(clicked()), this, SLOT(onToolboxButtonClicked()));
		}
		else if(bFound && iFoundIndex!=i)
		{
			bInvalidOrder = true;	
		}
			
	}

	if(bInvalidOrder) //recreate buttons list
	{
		m_lstToolboxButtons.clear();

		foreach(QAbstractButton * pButton, lstButtons)
		{
			m_lstToolboxButtons.append(pButton);
		}
	}
	
}

void MStyledToolBox::retranslate()
{
	for(int i=0;i<m_lstToolboxButtons.size();i++)
	{
		QAbstractButton * pBtn = m_lstToolboxButtons.at(i);
		pBtn->setText(tr(sGroupLabels.at(i).toLatin1().data()));
	}

	QList<QPushButton *> lstButtons =  this->findChildren<QPushButton *>();

	foreach(QPushButton * pButton, lstButtons)
	{
		pButton->setText(tr(pButton->objectName().toLatin1().data())); //use object name property as label to retranslate to other language

	}
}
