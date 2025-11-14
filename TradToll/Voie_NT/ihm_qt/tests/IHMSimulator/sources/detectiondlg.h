
#ifndef DENTECTIONDLG_H
#define DENTECTIONDLG_H

#include "Ui_DetectionDlg.h"
#include "IHMSimuDld.h"


class DetectionDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	DetectionDlg(QWidget *parent, QString szMailbox);

public slots:
	void accept();
	void reject();
	
private slots:

protected:
	virtual bool ReadXMLData();
	bool ReadRegValues();

	
private:
    Ui_DetectionDlg ui;
};

#endif

