

#include <QTableView>
#include <QKeyEvent>
#include <QHeaderView>
#include <QToolTip>
#include <QPainter>
#include <QTextDocument>
#include <QLabel>
#include <QScrollBar>
#include "MTracer.h"

#include "MTableView.h"
#include "MTableModel.h"
#include "MIhmConfigActions.h"
#include "MIhmConfigGeneral.h"


MTableView::MTableView(QWidget *parent)
    : QTableView(parent)
{
	m_iMemCurentRow = 0;
	m_bCanGetFocus = false;
	m_bIsInReadOnlyState = true;
	setSortingEnabled(false);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	horizontalHeader()->setStretchLastSection(true);
	horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	QScrollBar * scrlBarV = this->verticalScrollBar();
	if(scrlBarV!=NULL)
		scrlBarV->setContextMenuPolicy(Qt::NoContextMenu);
	QScrollBar * scrlBarH = this->horizontalScrollBar();
	if(scrlBarH!=NULL)
		scrlBarH->setContextMenuPolicy(Qt::NoContextMenu);

	verticalHeader()->hide();
	setAlternatingRowColors(true);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onClicked(const QModelIndex &)));
	connect(this, SIGNAL(entered(const QModelIndex &)), this, SLOT(onEntered(const QModelIndex &)));

	setMouseTracking(true);
	viewport()->setAttribute(Qt::WA_Hover,true);

	//to be able to get mouse move events from the header
	horizontalHeader()->viewport()->setMouseTracking(true);
	horizontalHeader()->viewport()->installEventFilter(this);
	
	m_delegate = new MTableViewDelegate;
	setItemDelegate(m_delegate);
}

MTableView::~MTableView()
{
	delete m_delegate;
}

//Used to handle mouse move events from the header
//and to override scrollbars right clicks
bool MTableView::eventFilter(QObject *obj,QEvent *pEvent)
{
	if(pEvent->type()==QEvent::MouseMove)
	{
		if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
			this->setCursor(Qt::BlankCursor);
		else
			this->setCursor( QCursor( Qt::ArrowCursor ));
	}

	return QObject::eventFilter(obj, pEvent);
}


void MTableView::setReadOnly(bool bReadOnly)
{
	m_bIsInReadOnlyState = bReadOnly;

	setCanGetFocus(m_bCanGetFocus);
}



//Use this event not to update cursor on every mouse move
void MTableView::onEntered(const QModelIndex & index)
{

	if(!m_bIsInReadOnlyState)
	{
		QVariant varAction = model()->data(index, MTableModel::ActionRole);
		QString sAction = varAction.toString();

		if(!sAction.isEmpty())
		{
			MIhmConfigActions *pCfgActions = MIhmConfigGeneral::getCfg()->getActions();
			QString sCursorId = pCfgActions->getCursorIdForAction(sAction);
			
			if(sCursorId!="")
			{
				QCursor oCursor = MIhmConfigGeneral::getCfg()->getCursor(sCursorId);
				this->setCursor(oCursor);
			}
			else
			{
				if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
					this->setCursor(Qt::BlankCursor);
				else
					this->setCursor(QCursor(Qt::PointingHandCursor));
			}
		}
		else
		{
			if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
				this->setCursor(Qt::BlankCursor);
			else
				this->setCursor(QCursor(Qt::ArrowCursor));
		}
	}
	
}


void MTableView::leaveEvent(QEvent * pEvent)
{
	if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
		this->setCursor(Qt::BlankCursor);
	else
		this->setCursor(QCursor(Qt::ArrowCursor));

	QToolTip::hideText();

}



void MTableView::focusInEvent(QFocusEvent * event)
{
	if(m_iMemCurentRow >= 0)
	{
		QItemSelectionModel *selectionModel = this->selectionModel();

		QModelIndex topLeft;
		QModelIndex bottomRight;
		int iColCount = this->model()->columnCount();

		topLeft = this->model()->index(m_iMemCurentRow, 0, QModelIndex());
		bottomRight = this->model()->index(m_iMemCurentRow, iColCount-1, QModelIndex());
		
		setCurrentIndex(topLeft);

		QItemSelection selection(topLeft, bottomRight);
		selectionModel->select(selection, QItemSelectionModel::Select);
	}
}

bool MTableView::onKeyDetected(int iDetectedKey)
{
	bool bRetVal = false;

	if(!m_bIsInReadOnlyState)
	{
		QItemSelectionModel *selectionModel = this->selectionModel();
		QModelIndexList indexes = selectionModel->selectedIndexes();

		if(indexes.count()>0)
		{
			int iCurrentRow = indexes.at(0).row();
			int iColCount = model()->columnCount(QModelIndex());

			QVariant varCurData;
				
			for(int iCol=0;iCol<iColCount; iCol++)
			{
				varCurData = model()->headerData(iCol, Qt::Horizontal, MTableModel::ActionKeyCode);

				if(iDetectedKey== varCurData.toInt())
				{
					QVariant varAction, varActionParams;
					QModelIndex cellIndex = model()->index(iCurrentRow, iCol, QModelIndex());
					
					emit clicked(cellIndex);
					bRetVal = true;
					break;
				}
			}
		}
	}

	return bRetVal;

}

void MTableView::forceFocus()
{
	if(m_bCanGetFocus && !m_bIsInReadOnlyState)
	{
		setFocusPolicy(Qt::StrongFocus);
		setFocus();
	}
}

void MTableView::setCanGetFocus(bool bEnableFocus)
{ 
	m_bCanGetFocus = bEnableFocus; 

	if(m_bCanGetFocus && !m_bIsInReadOnlyState)
	{
		setSelectionMode(QAbstractItemView::SingleSelection);
	}
	else
	{
		setSelectionMode(QAbstractItemView::NoSelection);
		setFocusPolicy(Qt::NoFocus);
	}
}


void MTableView::focusOutEvent(QFocusEvent * event)
{
	QItemSelectionModel *selectionModel = this->selectionModel();
	
	QModelIndex index = currentIndex();

	if(index.isValid())
	{
		m_iMemCurentRow = index.row();	
	}
	else
	{
		m_iMemCurentRow = -1;
	}
	
	selectionModel->clear();

	setFocusPolicy(Qt::NoFocus);
}




void MTableView::onClicked(const QModelIndex & index )
{
	if(m_bCanGetFocus && !m_bIsInReadOnlyState)
	{
		setFocusPolicy(Qt::StrongFocus);

		setCurrentIndex(index);

		if(index.isValid())
		{
			m_iMemCurentRow = index.row();	
		}
		else
		{
			m_iMemCurentRow = -1;
		}	

		setFocus();
	}
}

void MTableView::keyPressEvent( QKeyEvent * event )
{
	int iCode = event->nativeScanCode();
	int iCodeV = event->nativeVirtualKey();
	int iKey = event->key();
	QString s = event->text();
	unsigned char cAscii = 0;
	if(s.size()>0) cAscii = (unsigned char)s.at(0).toLatin1();

   switch(iKey)
   {
		case Qt::Key_Escape:
				clearFocus();
			break;

		default:
			QTableView::keyPressEvent(event);
			break;
   }

   event->accept();
   return;
}


 
void MTableViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{

 //   QStyleOptionViewItemV4 opt = setOptions(index, option);
 //   QVariant value;
 //
	//value = index.data(MTableModel::FontStyleBoldRole);
	//opt.font.setBold(value.toBool());

	//value = index.data(MTableModel::FontStyleStrikeOutRole);
	//opt.font.setStrikeOut(value.toBool());

	QItemDelegate::paint(painter, option, index);
}
 
void MTableViewDelegate::drawDisplay ( QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect, const QString & text ) const
{
	QItemDelegate::drawDisplay(painter, option, rect, text);
}

 
