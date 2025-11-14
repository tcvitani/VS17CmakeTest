
#ifndef M_LISTWIDGET_H
#define M_LISTWIDGET_H

#include <QListWidget>

class MIhmListWidget : public QListWidget
{
	    Q_OBJECT

public:
	MIhmListWidget(QWidget *parent = 0);
	~MIhmListWidget();

protected:
	virtual void keyPressEvent(QKeyEvent *event);

private:
};


#endif