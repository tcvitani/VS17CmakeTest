
#ifndef MTABLE_MODEL_VIEW_H
#define MTABLE_MODEL_VIEW_H

#include <QDomDocument>
#include <QPushButton>
#include "MTableModel.h"
#include "MTableView.h"

class QHBoxLayout;
class QFrame;
class QLabel;

class MTableModelView: public QObject
{
    Q_OBJECT

public:
    MTableModelView();
	~MTableModelView();

	void setTableView(MTableView *pTableView);
	void setMaxRowsAllowed(int iMaxRows);
	MTableView * getTableView(){return m_pTableView;};

	void appendRows(QList <MTableRow*> *plstNewRows);
	void prependRows(QList <MTableRow*> *plstNewRows);

	void clearAllRows();
	bool loadFormatFile(QString sDataFile);

	bool isEnabled();

	void forceFocus();
	bool onKeyDetected(int iDetectedKey);
	void setReadOnly(bool bReadOnly);

signals:
	void action(QString sAction, QString sParams);

protected:
	QDomDocument m_xmlData;

public slots:
	void onSelectionChanged(const QItemSelection & itemSelected, const QItemSelection &itemDeselected);
	void onCellClicked(const QModelIndex & index );
	
private:
	bool OpenXMLFile(QString sDataFile);
	void adjustColumnWidths();
	void adjustRowHeights();

	bool m_bEnabled;
	bool m_bParamCanGetFocus;
	Qt::ScrollBarPolicy m_iVScrollBarPolicy;
	Qt::ScrollBarPolicy m_iHScrollBarPolicy;

	MTableModel m_model;
	MTableView *m_pTableView;
};


#endif

