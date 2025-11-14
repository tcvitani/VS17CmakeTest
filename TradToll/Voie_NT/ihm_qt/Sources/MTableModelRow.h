
#ifndef TABLEMODELROW_H
#define TABLEMODELROW_H

#include <QString>

#include <QList>




class MTableCell
{
public:	
	MTableCell();
	MTableCell(const class QString & sText,const class QString & sAction,const class QString & sActionParam,const class QString & sTooltip,int iTextAlignment, bool bBold, bool bStrikeOut);
	MTableCell& operator=(const MTableCell& right);
	bool operator==(const MTableCell& right);
	
	QString m_sText;
	QString m_sAction;
	QString m_sActionParam;
	QString m_sTooltip;
	int m_iTextAlignment;
	bool m_bFontBold;
	bool m_bFontStrikeOut;
};


class MTableRow
{
	
public:	
	MTableRow();
	~MTableRow();
	MTableRow& operator=(const MTableRow& right);
	bool operator==(const MTableRow& right);

	static void copyTableRowList(QList <MTableRow*> *plstSource, QList <MTableRow*> *plstDest);
	
	QList <MTableCell *> m_lstColumns;
};


#endif
