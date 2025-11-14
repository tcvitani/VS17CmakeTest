
#include <MCtlList.h>
#include <QLabel.h>
#include <QListWidget.h>
#include <QApplication>


#include <MInputDialogReq.h>
#include <MHelpFuncs.h>
#include <MIhmConfigGeneral.h>
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
}


MCtlList::MCtlList(QWidget *parent)
{
	m_parentWidget = parent;

	m_lblCaption = new QLabel(m_parentWidget);
	m_listWidget = new QListWidget(m_parentWidget);
	m_font = QApplication::font();
	m_iNumItems = 0;
}

MCtlList::~MCtlList()
{
	delete m_lblCaption;
	delete m_listWidget;
}

void MCtlList::setFont( const QFont & fnt)
{
	m_font = fnt;
	updateFont();
}

void MCtlList::setFontSize(int iSize)
{
	if(iSize<=0)
		return;
	
	m_font.setPixelSize(iSize);

	updateFont();
}

void MCtlList::updateFont()
{
	if (m_lblCaption!=NULL) 
		m_lblCaption->setFont(m_font);

	if(m_listWidget!=NULL)
		m_listWidget->setFont(m_font);
			
}


void MCtlList::setCaption(QString sCaption)
{
	MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();

	pLang->getLabelTranslation(sCaption, MIhmLanguages::enuTranslTargetDesktop, m_sLabelText);
	
	m_lblCaption->setText(m_sLabelText);
	
	if(m_sLabelText=="")
		m_lblCaption->setVisible(false);
	else	
		m_lblCaption->setVisible(true);

}

bool MCtlList::addListItemDesc(QString sDesc)
{
	bool bOK = true;
	QString sTemp;
	MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();

	QString sUserInfo = MHelpFuncs::getStringItem(sDesc,0, QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);
	
	if(sUserInfo =="")
	{
		TRACE_W(QString("MCtlEdit::setDescription: Error decoding sUserInfo!"));
		return false;
	}
	
	m_lstUserInfo.append(sUserInfo);
	m_iNumItems = m_iNumItems + 1;
	
	QString sListItem = MHelpFuncs::getStringItem(sDesc,1, QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);

	//if first char is '?' translate the text 
	if(sListItem.at(0) == MInputDialogReq::cSAISIE_DECODE_TEXTE)
	{
		sListItem = sListItem.mid(1);
		pLang->getLabelTranslation(sListItem, MIhmLanguages::enuTranslTargetDesktop, sListItem);
	}
	
	m_listWidget->addItem(sListItem);
	


	return bOK;
}	



int MCtlList::getWidth()
{
	int iMaxWidth = 0;
	int iWidth;
	QString sTemp;
	int labelWidthInPixels;

	QFontMetrics fm(m_font);
	labelWidthInPixels = fm.size(Qt::TextSingleLine,m_lblCaption->text()).width();

	for (int i=0;i<m_listWidget->count();i++)
	{
		sTemp = m_listWidget->item(i)->text();
		iWidth = fm.size(Qt::TextSingleLine, sTemp).width();
		iMaxWidth = qMax(iMaxWidth,iWidth);
	}

	iMaxWidth = qMax(labelWidthInPixels,iMaxWidth);

	return iMaxWidth;
}


int MCtlList::getHeight()
{
	int iHeight;

	QFontMetrics fm(m_font);
	iHeight = (m_iNumItems + 1.8) * fm.height() * 1.2 ; //+ 1 because of the caption label

	return iHeight;
}


bool MCtlList::getSelection(QString &sText, QString &sData)
{
	int iRow = m_listWidget->currentRow();

	if(iRow>=0)
	{
		sData = m_lstUserInfo.at(iRow);
		sText = m_listWidget->item(iRow)->text();
		return true;
	}
	return false;
}


void MCtlList::setColors(QString sCaptionBack,
					QString sCaptionFore,
					QString sEditFore,
					QString sEditBack)
{
	QString sStyle;
	
	m_sCaptionBack = sCaptionBack;
	m_sCaptionFore = sCaptionFore;
	m_sEditFore = sEditFore;
	m_sEditBack = sEditBack;
	
	if(m_sCaptionBack!="")
		sStyle += MIhmConfigColor::createBkgColorStyle(sCaptionBack);

	if(m_sCaptionFore!="")
		sStyle += MIhmConfigColor::createColorStyle(m_sCaptionFore);

	m_lblCaption->setStyleSheet(sStyle);

	sStyle = "";

	if(m_sEditFore!="")
			sStyle += MIhmConfigColor::createColorStyle(m_sEditFore);

	if(m_sEditBack!="")
			sStyle += MIhmConfigColor::createBkgColorStyle(m_sEditBack);
	else
			sStyle += MIhmConfigColor::createBkgColorStyle("FFFFFF");

	m_listWidget->setStyleSheet(sStyle);
}
