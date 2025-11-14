
#ifndef MTABLE_VIEW_H
#define MTABLE_VIEW_H

#include <QTableView>
#include <QItemDelegate>


class QKeyEvent;
class MTableViewDelegate;

class MTableView: public QTableView
{
    Q_OBJECT

public:
    MTableView(QWidget *parent = 0);
	~MTableView();

	void setCanGetFocus(bool bEnableFocus);
	void forceFocus();
	bool onKeyDetected(int iDetectedKey);
	void setReadOnly(bool bReadOnly);

private slots:
	void onClicked(const QModelIndex &);
	void onEntered(const QModelIndex &);

protected:
	virtual void focusInEvent(QFocusEvent * event);
	virtual void focusOutEvent(QFocusEvent * event);
	virtual void keyPressEvent( QKeyEvent * event );
	virtual void leaveEvent(QEvent * event);
	bool eventFilter(QObject *obj,QEvent *pEvent);

private:
	int m_iMemCurentRow;
	bool m_bCanGetFocus;
	bool m_bIsInReadOnlyState;
	MTableViewDelegate * m_delegate; 
};

class MTableViewDelegate : public QItemDelegate
{
	Q_OBJECT
	
	protected:
		void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
		void drawDisplay ( QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect, const QString & text ) const;
};

#endif

