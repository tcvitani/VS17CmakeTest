
#ifndef LINKDLG_H
#define LINKDLG_H

#include "Ui_LinkDlg.h"
#include "IHMSimuDld.h"


class LinkDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	LinkDlg(QWidget *parent, QString szMailbox);

public slots:
	void accept();
	void reject();
	
private slots:

protected:
	virtual bool ReadXMLData();
	bool ReadRegValues();


private:
    Ui_LinkDlg ui;
};

#endif

