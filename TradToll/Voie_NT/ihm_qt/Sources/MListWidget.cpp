#include <QLabel>
#include <QScrollBar>
#include <QPixmapCache>

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}

#include "MListWidget.h"
#include "MIhmLanguages.h"
#include "MIhmConfigGeneral.h"
#include "MTracer.h"


MListWidget::MListWidget(QWidget *parent)
    : QListWidget(parent)
{
	setFrameShape(QFrame::NoFrame);
	setSelectionMode(QAbstractItemView::NoSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setDragEnabled(false);
	setMouseTracking(true);
	setFocusPolicy(Qt::NoFocus);
	
	QScrollBar * scrlBarV = this->verticalScrollBar();
	if(scrlBarV!=NULL)
		scrlBarV->setContextMenuPolicy(Qt::NoContextMenu);
	QScrollBar * scrlBarH = this->horizontalScrollBar();
	if(scrlBarH!=NULL)
		scrlBarH->setContextMenuPolicy(Qt::NoContextMenu);


	m_pToolTipLabel = NULL;
	m_filterObj = NULL;


}

MListWidget::~MListWidget()
{

}

void MListWidget::installEventFilter(QObject * filterObj)
{
	m_filterObj = filterObj;
	QListWidget::installEventFilter(m_filterObj);
	//QTextEdit and QListView inherit from QAbstractScrollArea.
	// 	You need to install the filter for the viewport()
	// 	of the scroll area to filter the events target to QList View
	this->viewport()->installEventFilter(m_filterObj);

	
	if(m_pToolTipLabel!=NULL)
	{
		m_pToolTipLabel->setMouseTracking(true);
		m_pToolTipLabel->installEventFilter(m_filterObj);
	}
}


void MListWidget::clear()
{
	if (m_pToolTipLabel != NULL)
		m_pToolTipLabel->setText("");
		
	m_sLastItem = "";
	((QListWidget*)this)->clear();

}

void MListWidget::initialize(QLabel *pToolTipLabel, bool bShowText, bool bShowIcons, bool bTooltipShowLastItem)
{
	m_bShowText = bShowText;
	m_bShowIcons = bShowIcons;
	m_bTooltipShowLastItem = bTooltipShowLastItem;

	m_pToolTipLabel = pToolTipLabel;

	if (m_pToolTipLabel != NULL)
	{
		QObject::connect(this, SIGNAL(itemEntered(QListWidgetItem *)), this, SLOT(onItemEntered(QListWidgetItem *)));
	}
}

void MListWidget::updateToolTip()
{
	if(m_pToolTipLabel==NULL)
		return;
	else 
	{
		QPoint pt = QCursor::pos();

		QListWidgetItem * item = itemAt(mapFromGlobal(pt));

		if (item != NULL)
		{
			// get the tooltip text
			QString sItemData = item->data(Qt::UserRole).toString();

			MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
			QString sTranslation;
			pLang->getLabelTranslation(sItemData, MIhmLanguages::enuTranslTargetNoTransform, sTranslation);
			m_pToolTipLabel->setText(sTranslation);
		}
		else if (m_bTooltipShowLastItem)
			updateToolTipToLastItem();
		else
			m_pToolTipLabel->setText("");
	}

}


void MListWidget::updateToolTipToLastItem()
{
	QString sTranslation;
	MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	pLang->getLabelTranslation(m_sLastItem, MIhmLanguages::enuTranslTargetNoTransform, sTranslation);
	m_pToolTipLabel->setText(sTranslation);

}

//Slot used for the alarm lists tooltip handling
void MListWidget::onItemEntered(QListWidgetItem * item)
{
	UNREFERENCED_PARAMETER(item);
	
	if (m_pToolTipLabel != NULL)
		updateToolTip();
}


void MListWidget::leaveEvent(QEvent * event)
{
	QListWidget::leaveEvent(event);
	
	if (m_pToolTipLabel != NULL)
	{
		if (m_bTooltipShowLastItem)
			updateToolTipToLastItem();
		else
			m_pToolTipLabel->setText("");
	}

}

bool MListWidget::loadIconPixmap(QPixmap &oPixmap, QString sImgPath)
{
	 bool bPixmapOk = false;
	 MIhmConfigImages * pImages = MIhmConfigGeneral::getCfg()->getDskConfigImages();
	 QPixmap oPix;

	 //first ty to find in cash  
	 if(sImgPath.size()>0)
	 {
		 if (QPixmapCache::find(sImgPath, &oPix))
		 {
			 oPixmap = oPix;
			 bPixmapOk = true;
		 } 
		 else 
		 {
			 //if not found in cash load it from disk and add it to the cash
			 if(oPixmap.load(sImgPath))
			 {
				 QPixmapCache::insert(sImgPath, oPixmap);
				 bPixmapOk = true;
			 }
		 }

		 if(!bPixmapOk)
			TRACE_W(QString("MFormMain::loadIconPixmap: Unable to load image %1").arg(sImgPath));
	 }	
	 
	 if(!bPixmapOk)
	 {
		 //---------------------------------------------------------
		 //if the correct list item  image was not found show the default image
		 sImgPath = pImages->getImageFullPath(IHM_IMG_DEFAULT);
		 
		 //-----------------------------------------------
		 //try to find the default image in cash
		 if (QPixmapCache::find(sImgPath, &oPixmap))
		 {
			 bPixmapOk = true;
		 } 
		 else 
		 {
			 //if not found in cash load it from disk and add it to the cash
			 if(oPixmap.load(sImgPath))
			 {
				 QPixmapCache::insert(sImgPath, oPixmap);
				 bPixmapOk = true;
			 }
		 }
		 
		 //------------------------------------------------
		 if(!bPixmapOk)
		 {
			 TRACE_W(QString("MFormMain::loadIconPixmap: Unable to load image %1")
				 .arg(sImgPath));
		 }
	}

	return bPixmapOk;
}


void MListWidget::addItemsToListView(QStringList &lstToAdd)
{

	if(lstToAdd.size()==0)
		 return;

	QString sItemToAdd;
 
	for (int i=0;i<lstToAdd.size();++i)
	{
		sItemToAdd = lstToAdd.at(i);
		QListWidgetItem *pNewItem = new QListWidgetItem();
		pNewItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		pNewItem->setData(Qt::UserRole, sItemToAdd);


		if(m_bShowText)
		{
			QString sTranslation;
			MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
			pLang->getLabelTranslation(sItemToAdd, MIhmLanguages::enuTranslTargetNoTransform, sTranslation);
			pNewItem->setText(sTranslation);
		}

		if(m_bShowIcons)
		{
			MIhmConfigImages * pImages = MIhmConfigGeneral::getCfg()->getDskConfigImages();
			QString sImgPath = pImages->getImageFullPath(sItemToAdd);
			
			if(sImgPath.size()==0)
			{
				TRACE_W(QString("MListWidget::addItemsToListView: Unable to find icon for %1")
					.arg(sItemToAdd));
			}
			
			QPixmap oPixmap;
			QIcon icon;
			
			if(loadIconPixmap(oPixmap, sImgPath)==true)
			{
				icon.addPixmap(oPixmap);
				pNewItem->setIcon(icon);
			}	
		}

		this->insertItem(0, pNewItem);
	}

	
	if (m_pToolTipLabel != NULL )
	{
		m_sLastItem = sItemToAdd;
		updateToolTip();
	}



}