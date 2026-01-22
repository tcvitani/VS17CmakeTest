#include <QtGui>


#include "MStyledLangPick.h"



MStyledLangButton::MStyledLangButton(QWidget *parent, QString sUnselectedImagePath, QString sSelectedImagePath, QString sTextToDisplay, QString sID)
	:QFrame(parent)
{
    setObjectName(QString::fromUtf8("frLang"));
    setMinimumSize(QSize(28, 18));
    setMaximumSize(QSize(28, 16777215));
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Raised);
    setLineWidth(0);
    QVBoxLayout * verticalLayout_5 = new QVBoxLayout(this);
    verticalLayout_5->setSpacing(0);
    //verticalLayout_5->setMargin(0);
    verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));

	m_lblFlag = new QLabel(this);
    m_lblFlag->setObjectName(QString::fromUtf8("m_lblFlag"));
    m_lblFlag->setMinimumSize(QSize(27, 19));
    m_lblFlag->setMaximumSize(QSize(27, 19));
    m_lblFlag->setScaledContents(true);
    m_lblFlag->setCursor(QCursor(Qt::PointingHandCursor));

	m_pColPixmap = new QPixmap(sUnselectedImagePath);
	m_pMonoPixmap = new QPixmap(sSelectedImagePath);

    m_lblFlag->setPixmap(*m_pColPixmap);
    m_lblFlag->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

    verticalLayout_5->addWidget(m_lblFlag);

    m_lblLang = new QLabel(this);
    m_lblLang->setObjectName(QString::fromUtf8("m_lblLang"));
	m_lblLang->setText(sTextToDisplay);
    m_lblLang->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    m_lblLang->setCursor(QCursor(Qt::PointingHandCursor));

    verticalLayout_5->addWidget(m_lblLang);

	m_sID = sID;
}


MStyledLangButton::~MStyledLangButton()
{
	delete m_pColPixmap;
	delete m_pMonoPixmap;
}

void MStyledLangButton::setSelected(bool bSelected)
{
	if(bSelected)   
		m_lblFlag->setPixmap(*m_pMonoPixmap);
	else
		m_lblFlag->setPixmap(*m_pColPixmap);

}

void MStyledLangButton::mouseReleaseEvent(QMouseEvent * event)
{
	emit clicked(m_sID);
	QFrame::mouseReleaseEvent(event);
}
//-------------------------------------------------------

MStyledLangPick::MStyledLangPick(QWidget *parentFrame)
{
	m_parentFrame = parentFrame;
	
	m_horizontalLayout = new QHBoxLayout(this);
    m_horizontalLayout->setSpacing(3);
   // m_horizontalLayout->setMargin(0);
    m_horizontalLayout->setObjectName(QString::fromUtf8("m_horizontalLayout"));
	m_horizontalLayout->setDirection(QBoxLayout::LeftToRight);

	m_bStateSelectingInProgress = false;
}

void MStyledLangPick::setAlignment(Qt::Alignment alignment)
{
	if(alignment == Qt::AlignRight)
	{
		m_horizontalLayout->insertStretch(0);
	}
	else
	{
		m_horizontalLayout->addStretch();
	}
}

void MStyledLangPick::appendLanguage(QString sColourImagePath, QString sMonoImagePath, QString sTextToDisplay, QString sLangID)
{

	MStyledLangButton * pLangButton = new MStyledLangButton(this, sColourImagePath, sMonoImagePath, sTextToDisplay, sLangID);
    connect(pLangButton, SIGNAL(clicked(QString)), this, SLOT(onLangSelected(QString)));
	m_horizontalLayout->addWidget(pLangButton);

	m_lstButtons.append(pLangButton);

	if(m_lstButtons.size()==1)
		setSelectedLang(sLangID);

	displaySelectedState();
}

void MStyledLangPick::onLangSelected(QString sLangID)
{
	if(!m_bStateSelectingInProgress)
	{
		displaySelectingState();
	}
	else
		setSelectedLang(sLangID);
}


void MStyledLangPick::setSelectedLang(QString sLangID)
{
	m_sSelectedLangID = sLangID;
	emit selectionChanged(m_sSelectedLangID);

	displaySelectedState();
}


void MStyledLangPick::displaySelectedState()
{
	m_bStateSelectingInProgress = false;

	for(int i=0; i<m_lstButtons.size();  i++)	
	{
		MStyledLangButton * p = m_lstButtons.at(i);
		
		if(p->getId()==m_sSelectedLangID)
		{
			p->setSelected(true);
			p->show();
		}
		else
		{
			p->hide();
		}
	}
}

void MStyledLangPick::displaySelectingState()
{
	m_bStateSelectingInProgress = true;

	for(int i=0; i<m_lstButtons.size();  i++)	
	{
		MStyledLangButton * p = m_lstButtons.at(i);
		
		p->setSelected(false);
		p->show();
	}
}

void MStyledLangPick::toogleSelectingState()
{
	if(m_bStateSelectingInProgress)
		displaySelectedState();
	else
		displaySelectingState();
}
