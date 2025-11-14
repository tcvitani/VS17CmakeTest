
#include <QHBoxLayout>
#include <QtUiTools>
#include <QFile>
#include <QFrame>
#include <QTableWidget>
#include <QLabel>
#include <QHeaderView>
#include <QScrollBar>
#include <MHelpFuncs.h>
#include <QLineEdit>

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <ihm.h>
	#include "ihm_loc.h"
}

#include "MTableModel.h"
#include "MTableModelRow.h"
#include "MTableModelView.h"
#include "MTracer.h"


MTableModelView::MTableModelView()
{
	m_bEnabled = false;
}

MTableModelView::~MTableModelView()
{

}

void MTableModelView::setTableView(MTableView *pTableView)
{
	m_pTableView = pTableView;	
	m_pTableView->setModel(&m_model);

	m_pTableView->setCanGetFocus(m_bParamCanGetFocus);

	m_pTableView->setHorizontalScrollBarPolicy(m_iHScrollBarPolicy);
	m_pTableView->setVerticalScrollBarPolicy(m_iVScrollBarPolicy);

	connect(m_pTableView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
				this, SLOT(onSelectionChanged(const QItemSelection &, const QItemSelection &)));

	connect(m_pTableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onCellClicked(const QModelIndex &)));

	adjustRowHeights();
	adjustColumnWidths();

}

void MTableModelView::setMaxRowsAllowed(int iMaxRows)
{
	m_model.setMaxRowsAllowed(iMaxRows);
}


void MTableModelView::onCellClicked(const QModelIndex & index )
{
	QVariant varAction, varActionParams;
	varAction = m_model.data(index, MTableModel::ActionRole);
	varActionParams = m_model.data(index, MTableModel::ActionParamRole);

	emit action(varAction.toString(), varActionParams.toString());
}


bool MTableModelView::onKeyDetected(int iDetectedKey)
{
	bool bRetVal = false;

	if(m_pTableView->hasFocus())
	{
		bRetVal = m_pTableView->onKeyDetected(iDetectedKey);
	}

	return bRetVal;
}

void MTableModelView::forceFocus()
{
	if(m_pTableView!=NULL)
	{
		m_pTableView->forceFocus();
	}
}	


void MTableModelView::setReadOnly(bool bReadOnly)
{
	if(m_pTableView!=NULL)
	{
		m_pTableView->setReadOnly(bReadOnly);
	}
}


void MTableModelView::adjustColumnWidths()
{
	int iColCount = m_model.columnCount(QModelIndex());
	int iCurColWidth;
	QVariant curVar;

	for (int i=0;i<iColCount;i++)
	{
		curVar = m_model.headerData(i,Qt::Horizontal, MTableModel::ColumnWidthRole);
		
		iCurColWidth = curVar.toInt();

		if(iCurColWidth!=0)
			m_pTableView->setColumnWidth(i,iCurColWidth);
	}

	//adjust header height...
	int iHeight = m_model.getHeaderHeight();
	
	if(iHeight>0)
	{
		QHeaderView	*pView = m_pTableView->horizontalHeader();
		pView->setMinimumHeight(iHeight);
		pView->setMaximumHeight(iHeight);
	}
}

void MTableModelView::adjustRowHeights()
{
	m_pTableView->verticalHeader()->setDefaultSectionSize(m_model.getRowHeight());	
}



void MTableModelView::onSelectionChanged(const QItemSelection & itemSelected, const QItemSelection &itemDeselected)
{
	QModelIndex index;
	QVariant cellData;

	//to do: see if needed or it can be found  
	foreach(index, itemSelected.indexes())
	{
		cellData = m_model.data(index, Qt::DisplayRole);
	}

}



bool MTableModelView::isEnabled()
{
	return m_bEnabled;
}


bool MTableModelView::loadFormatFile(QString sDataFile)
{
	bool bRet = false;
	QString sTemp;

	if(OpenXMLFile(sDataFile))			
	{
		QDomElement root = m_xmlData.documentElement();
		QDomElement elBody = root.firstChildElement(IHM_CFG_VAL_TABLEVIEW_BODY_ELEMENT);
		QDomNodeList lstRows = elBody.elementsByTagName(IHM_CFG_VAL_TABLEVIEW_ROW_ELEMENT);
		
		QString sRowHeight = elBody.attribute(IHM_CFG_VAL_TABLEVIEW_HEIGHT_ATTRIBUTE);
		bool bOK;
		int iRowHeight = sRowHeight.toInt(&bOK);
		m_model.setRowHeight(iRowHeight);

		sTemp = elBody.attribute(IHM_CFG_VAL_TABLEVIEW_CAN_GET_FOCUS_ATTRIBUTE); 	
		if(sTemp.toInt()>0)
			m_bParamCanGetFocus = true;
		else
			m_bParamCanGetFocus = false;

		sTemp = elBody.attribute(IHM_CFG_VAL_TABLEVIEW_VSCROLL_POLICY_ATTRIBUTE); 	
		m_iVScrollBarPolicy = (Qt::ScrollBarPolicy)sTemp.toInt();
		if(m_iVScrollBarPolicy>Qt::ScrollBarAlwaysOn)
			m_iVScrollBarPolicy = Qt::ScrollBarAsNeeded;

		sTemp = elBody.attribute(IHM_CFG_VAL_TABLEVIEW_HSCROLL_POLICY_ATTRIBUTE); 	
		m_iHScrollBarPolicy = (Qt::ScrollBarPolicy)sTemp.toInt();
		if(m_iHScrollBarPolicy>Qt::ScrollBarAlwaysOn)
			m_iHScrollBarPolicy = Qt::ScrollBarAsNeeded;


		QString sColWidth,sHeaderHeight, sHeaderResize ,sAlign;
		QDomElement elHeader = root.firstChildElement(IHM_CFG_VAL_TABLEVIEW_HEADER_ELEMENT);
		
		sHeaderHeight = elHeader.attribute(IHM_CFG_VAL_TABLEVIEW_HEIGHT_ATTRIBUTE);
		int iHeaderHeight = sHeaderHeight.toInt(&bOK);
		
		m_model.setHeaderHeight(iHeaderHeight);
		
		QDomNode currColumn; 
		QString sText, sActionKeyCode;
		QDomNodeList lstHeadColumns = elHeader.elementsByTagName(IHM_CFG_VAL_TABLEVIEW_COLUMN_ELEMENT);
		int iColWidth,iActionKeyCode;

		MTableHeaderCell * pNewHeaderCell;

		for(int i=0;i<lstHeadColumns.count();i++)
		{
			currColumn = lstHeadColumns.at(i);
			sText = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_TEXT_ATTRIBUTE);
			sColWidth = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_WIDTH_ATTRIBUTE);
			sAlign = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_ALIGN_ATTRIBUTE);
			sActionKeyCode = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_ACTION_KEY_CODE_ATTRIBUTE);
			iActionKeyCode = sActionKeyCode.toInt(&bOK);

			iColWidth = sColWidth.toInt(&bOK);
			
			pNewHeaderCell = new MTableHeaderCell();
			pNewHeaderCell->sTextLabel = sText;

			pNewHeaderCell->iWidth = iColWidth;
			pNewHeaderCell->iContentAlign = MHelpFuncs::convertAlignment(sAlign);
		
     		pNewHeaderCell->iActionKeyCode = iActionKeyCode;

			m_model.addHeaderCell(pNewHeaderCell);
		}

		bRet = true;
	}
		
	return bRet;
}

void MTableModelView::prependRows(QList <MTableRow*> *plstNewRows)
{
	QList <MTableRow*> lstCopiedNewRows;
	MTableRow::copyTableRowList(plstNewRows, &lstCopiedNewRows);

	m_model.prependTableRows(&lstCopiedNewRows);
}
	
void MTableModelView::appendRows(QList <MTableRow*> *plstNewRows)
{
	QList <MTableRow*> lstCopiedNewRows;
	MTableRow::copyTableRowList(plstNewRows, &lstCopiedNewRows);

	m_model.addTableRows(&lstCopiedNewRows);
}

void MTableModelView::clearAllRows()
{
	m_model.clearTableRows();
}


bool MTableModelView::OpenXMLFile(QString sDataFile)
{
	bool bRet = false;
	QString errorStr;
	int errorLine;
	int errorColumn;
	QFile file(sDataFile);
	
	if(file.open(QFile::ReadOnly))
	{

		bRet = m_xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn);

		if(bRet)
		{
			QDomElement root = m_xmlData.documentElement();
			if(root.tagName() != IHM_CFG_VAL_TABLEVIEW_ROOT_ELEMENT)
			{
//				IhmDebugError( "MTableModelView::OpenXML: Invalid root element!");
				bRet = false;
			}
		}
		else
		{
			QString sMsg = QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
//	 		IhmDebugError( "MTableModelView::OpenXML: %s",sMsg.toLatin1().data() );
		}
	}
	else
	{
//		IhmDebugError( "MTableModelView::OpenXML: Unable to open file %s",sDataFile.toLatin1().data() );
	}
	return bRet;
}



