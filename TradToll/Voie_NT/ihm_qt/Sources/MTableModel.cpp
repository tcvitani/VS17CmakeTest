
#include "MIhmConfigGeneral.h"

#include "MTableModel.h"
#include "MTableModelRow.h"


MTableModel::MTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
	m_iMaxRowsAllowed = 0; //unlimited
}

MTableModel::~MTableModel()
{
	while(!m_lstHeaderCells.isEmpty())
		delete m_lstHeaderCells.takeFirst();

	while(!m_lstRows.isEmpty())
		delete m_lstRows.takeFirst();
}

int MTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_lstRows.size();
}

int MTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_lstHeaderCells.size();
}

QVariant MTableModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();
    
	int iRow = index.row();

    if (iRow >= m_lstRows.size() || iRow < 0)
        return QVariant();
       
    if ((index.column() >= m_lstRows.at(iRow)->m_lstColumns.size()) || 
			(index.column() < 0))
		return QVariant();
	else
	{	
		MTableCell * pCell = m_lstRows.at(iRow)->m_lstColumns.at(index.column());

		if (role == Qt::DisplayRole) 
		{
			MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();
			QString sTranslatedTextLabel;
			pLang->getLabelTranslation(pCell->m_sText, MIhmLanguages::enuTranslTargetDesktop, sTranslatedTextLabel);
			return sTranslatedTextLabel;	
		}
		if (role == Qt::ToolTipRole) 
		{
			MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();
			QString sTranslatedTextLabel;
			pLang->getLabelTranslation(pCell->m_sTooltip, MIhmLanguages::enuTranslTargetDesktop, sTranslatedTextLabel);
			return sTranslatedTextLabel;
		}
		if (role == Qt::TextAlignmentRole) 
		{
			return pCell->m_iTextAlignment;	
		}
		else if(role == ActionRole)
		{
			return pCell->m_sAction;
		}
		else if(role == ActionParamRole)
		{
			return pCell->m_sActionParam;
		}
		else if(role == FontStyleBoldRole)
		{
			return pCell->m_bFontBold;
		}
		else if(role == FontStyleStrikeOutRole)
		{
			return pCell->m_bFontStrikeOut;
		}
	
	}

    return QVariant();
}


QVariant MTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) 
	{
		if(m_lstHeaderCells.count()>section)
		{
			if (role == Qt::DisplayRole )
			{
				MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();
				QString sTranslatedTextLabel;
				pLang->getLabelTranslation(m_lstHeaderCells.at(section)->sTextLabel, MIhmLanguages::enuTranslTargetDesktop, sTranslatedTextLabel);

				return sTranslatedTextLabel;
			}
			else if (role == ColumnWidthRole )
			{
				return m_lstHeaderCells.at(section)->iWidth;
			}
			else if (role == ActionKeyCode )
			{
				return m_lstHeaderCells.at(section)->iActionKeyCode;
			}
		}
	}

    return QVariant();
}

bool MTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
	
	if(rows>0)
	{
		beginInsertRows(QModelIndex(), position, position+rows-1);
    
		for (int row=0; row < rows; row++) {
			MTableRow *pNewRow = new MTableRow();
			m_lstRows.insert(position, pNewRow);
		}

		endInsertRows();
	}

	correctTableSize();
    return true;
}


Qt::ItemFlags MTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}


void MTableModel::InitTableModel(QList <MTableHeaderCell*> *plstHeaderCells)
{
	while(!m_lstHeaderCells.isEmpty())
		delete m_lstHeaderCells.takeFirst();
	
	m_lstHeaderCells = *plstHeaderCells;

	emit headerDataChanged(Qt::Horizontal, 0, m_lstHeaderCells.count());
}

void MTableModel::setMaxRowsAllowed(int iMaxRows)
{
	m_iMaxRowsAllowed = iMaxRows;
}

void MTableModel::addHeaderCell(MTableHeaderCell* pHeaderCell)
{
	m_lstHeaderCells.append(pHeaderCell);
	
	emit headerDataChanged(Qt::Horizontal, 0, m_lstHeaderCells.count());
}

void MTableModel::addTableRows(QList <MTableRow*> *plstRows)
{
	int iPos = m_lstRows.count();
	int iCount = plstRows->count();
	
	if(iCount>0)
	{
		beginInsertRows(QModelIndex(), iPos, iPos + iCount - 1);
		m_lstRows.append(*plstRows);
		endInsertRows();
	}
	
	correctTableSize();
}

void MTableModel::prependTableRows(QList <MTableRow*> *plstRows)
{
	if(plstRows==NULL)
		return;
	
	int iCount = plstRows->count();
    
	if(iCount>0)
	{
		beginInsertRows(QModelIndex(), 0, iCount - 1);

		for(int i = 0; i < iCount; i++)
		{
			m_lstRows.prepend(plstRows->at(i));
		}

		endInsertRows();
	}

	correctTableSize();
}

void MTableModel::correctTableSize()
{
	//assure that the row count is not higher than configured in ConfigLaneTypes MAX_ROWS parameter for the object 
	
	if(m_iMaxRowsAllowed>0) // if not unlimited
	{
		if(m_lstRows.count()>m_iMaxRowsAllowed)
		{
			beginRemoveRows(QModelIndex(), m_iMaxRowsAllowed, m_lstRows.count()-1);
			
			while(m_lstRows.count()>m_iMaxRowsAllowed)
				delete m_lstRows.takeLast();
			
			endRemoveRows();
		}
		
	}
}

void MTableModel::clearTableRows()
{
	emit headerDataChanged(Qt::Horizontal, 0, m_lstHeaderCells.count());

	if(m_lstRows.count()>0)
	{
		beginRemoveRows(QModelIndex(), 0, m_lstRows.count()-1);
		
		while(!m_lstRows.isEmpty())
			delete m_lstRows.takeFirst();
		
		endRemoveRows();
	}
}

void MTableModel::setRowHeight(int iHeight)
{
	m_iRowHeight = iHeight;
}


void MTableModel::setHeaderHeight(int iHeight)
{
	m_iHeaderHeight = iHeight;
}
