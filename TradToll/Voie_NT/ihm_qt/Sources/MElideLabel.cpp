
#include <QResizeEvent>
#include "MElideLabel.h"


MElideLabel::MElideLabel(QWidget * parent):
	QWidget(parent)
{
	m_parent = parent;
	m_pLabel = new QLabel(this);
	m_pLabel->move(0,0);
	m_pLabel->setGeometry(this->geometry());
}

MElideLabel::~MElideLabel()
{
	delete m_pLabel;
}

void MElideLabel::setVisible(bool b)
{
	m_pLabel->setVisible(b);
	QWidget::setVisible(b);

}

void MElideLabel::setLabelProperties(QLabel *m_lblMenuTitle)
{
	m_pLabel->setFont(m_lblMenuTitle->font());
	this->setGeometry(m_lblMenuTitle->geometry());
	m_pLabel->setAlignment(m_lblMenuTitle->alignment());	
	this->setStyleSheet(m_lblMenuTitle->styleSheet());
	m_pLabel->setStyleSheet(m_lblMenuTitle->styleSheet());
}


void MElideLabel::updateText(int iNewWidth)
{
	QFont fnt = m_pLabel->font();
	QFontMetrics fm(fnt);	
	QString sElidedText;
	sElidedText = fm.elidedText ( m_sCompleteText, Qt::ElideRight, iNewWidth);
		
	m_pLabel->setText(sElidedText);
}


void MElideLabel::setText(QString sText)
{
	m_sCompleteText = sText;
	updateText(width());
}

void MElideLabel::setToolTip(QString sToolTip)
{
	m_pLabel->setToolTip(sToolTip);
}



void MElideLabel::resizeEvent(QResizeEvent * event)
{
	Q_UNUSED(event)

	updateText(width());
	m_pLabel->setGeometry(this->geometry());
	m_pLabel->move(0,0);
}

