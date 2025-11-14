

#include "MTableModelRow.h"
#include "MHelpFuncs.h"


MTableCell::MTableCell()
{

}

MTableCell::MTableCell(const class QString &sText,const class QString & sAction,const class QString & sActionParam,const class QString & sTooltip,int iTextAlignment, bool bBold, bool bStrikeOut):
	m_sText(sText), m_sAction(sAction), m_sActionParam(sActionParam),
	 m_sTooltip(sTooltip),m_iTextAlignment(iTextAlignment), m_bFontBold(bBold), m_bFontStrikeOut(bStrikeOut)
{

}

MTableCell& MTableCell::operator=(const MTableCell& right)
{
	m_sText = MHelpFuncs::deepCopy(right.m_sText);
	m_sAction = MHelpFuncs::deepCopy(right.m_sAction);
	m_sActionParam = MHelpFuncs::deepCopy(right.m_sActionParam);
	m_sTooltip = MHelpFuncs::deepCopy(right.m_sTooltip);
	m_iTextAlignment = right.m_iTextAlignment;
	m_bFontBold = right.m_bFontBold;
	m_bFontStrikeOut = right.m_bFontStrikeOut;
	
	return *this;
}

bool MTableCell::operator==(const MTableCell& right)
{
// 	bool bEqual = true;
// 
// 	bEqual = bEqual && (m_sText == m_sText);
// 	if(!bEqual) 
// 		return false;
	
//Note: for optimization purposes we compare only text fields

	return (m_sText == m_sText);
}



//----------------------------------------------------
	
MTableRow::MTableRow()
{

}

MTableRow::~MTableRow()
{
	while(!m_lstColumns.isEmpty())
			delete m_lstColumns.takeLast();
}

MTableRow& MTableRow::operator=(const MTableRow& right)
{
	MTableCell *pCurrentCell;
	MTableCell *pNewCell;

	for(int i=0; i< right.m_lstColumns.count();i++)
	{
		pCurrentCell = (MTableCell*)right.m_lstColumns.at(i);
		pNewCell = new MTableCell();

		*pNewCell = *pCurrentCell;	
		this->m_lstColumns.append(pNewCell);
	}
	
	return *this;
}

bool MTableRow::operator==(const MTableRow& right)
{
	bool bEqual = true;
	MTableCell* pCurrentLeft;
	MTableCell* pCurrentRight;

	for (int i=0; i< right.m_lstColumns.count(); i++)
	{
		pCurrentLeft = m_lstColumns.at(i);
		pCurrentRight = right.m_lstColumns.at(i);

		if(pCurrentLeft!=NULL && pCurrentRight!=NULL)
			bEqual = bEqual && (*pCurrentLeft == *pCurrentRight);
		else
			bEqual = false;

		if(!bEqual)
		{
			break;
		}
	}
	
	return bEqual;
}



void MTableRow::copyTableRowList(QList <MTableRow*> *plstSource, QList <MTableRow*> *plstDest)
{
	MTableRow * pNewRow = NULL;

	for (int i=0; i<plstSource->count(); i++)
	{
		pNewRow = new MTableRow();
		*pNewRow = *((MTableRow*)plstSource->at(i));
		plstDest->append(pNewRow);	
	}

}