
#ifndef MELIDE_LABEL_H
#define MELIDE_LABEL_H

#include <QLabel>


class MElideLabel:public QWidget
{
	Q_OBJECT
public:
	MElideLabel(QWidget * parent = 0);
	~MElideLabel();
	void setText(QString sText);
	void setToolTip(QString sToolTip);
	void setLabelProperties(QLabel *m_lblMenuTitle);
	
	QLabel *getLabel(){return m_pLabel;}

	void setVisible(bool b);
signals:
	void message(QString);

protected:
	void updateText(int iNewWidth);
	virtual void resizeEvent(QResizeEvent * event);
	
	QLabel *m_pLabel;
	QString m_sCompleteText;
	QWidget *m_parent;
};



#endif

