
#ifndef TOUCHEDLG_H
#define TOUCHEDLG_H

#include "Ui_touchedlg.h"
#include "IHMSimuDld.h"


class ToucheDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	ToucheDlg(QWidget *parent, QString sIHMMailbox);

public slots:
	void accept();
	void reject();
	
private slots:

protected:
	virtual bool ReadXMLData();
	bool ReadRegValues();

	
private:
    Ui_ToucheDlg ui;
};

#endif

