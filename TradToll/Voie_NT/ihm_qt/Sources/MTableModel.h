
#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QString>
#include <QList>

class MTableHeaderCell
{
public:	
	QString sTextLabel;
	//int iHeaderAlign;
	int iContentAlign;
	int iWidth;
	int iActionKeyCode;
};

class MTableRow;

class MTableModel : public QAbstractTableModel
{
    Q_OBJECT
    
public:
	enum {
		ColumnWidthRole = Qt::UserRole,
		FontStyleBoldRole,
		FontStyleStrikeOutRole,
		ActionRole,
		ActionParamRole,
		ActionKeyCode,

	} MyDataRoles;


    MTableModel(QObject *parent=0);
    ~MTableModel();

	void InitTableModel(QList <MTableHeaderCell*> *plstHeaderCells);
	void setMaxRowsAllowed(int iMaxRows);
	void addHeaderCell(MTableHeaderCell* pHeaderCell);
	void addTableRows(QList <MTableRow*> *plstRows);
	void prependTableRows(QList <MTableRow*> *plstRows);
	void clearTableRows();
	void setRowHeight(int iHeight);
	void setHeaderHeight(int iHeight);
	int getRowHeight() const { return m_iRowHeight;};
	int getHeaderHeight() const { return m_iHeaderHeight;};

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
	
private:
	void correctTableSize();
	QList <MTableRow*> m_lstRows;
	QList <MTableHeaderCell*> m_lstHeaderCells;
	int m_iRowHeight;
	int m_iHeaderHeight;
	int m_iMaxRowsAllowed;
};


#endif
