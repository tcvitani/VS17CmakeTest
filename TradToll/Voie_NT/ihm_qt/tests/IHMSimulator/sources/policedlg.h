
#ifndef POLICEDLG_H
#define POLICEDLG_H

#include "Ui_PoliceDlg.h"
#include "IHMSimuDld.h"


class PoliceDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	PoliceDlg(QWidget *parent, QString sIHMMailbox);

public slots:
	void accept();
	void reject();
	
private slots:

protected:
	virtual bool ReadXMLData();
	bool ReadRegValues();

	
private:
    Ui_PoliceDlg ui;
};

#endif

