
#ifndef SAISIEEXDLG_H
#define SAISIEEXDLG_H

#include "Ui_SaisieExDlg.h"
#include "IHMSimuDld.h"


class SaisieExDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	SaisieExDlg(QWidget *parent, QString sIHMMailbox);

public slots:
	void accept();
	void reject();
	void spinChanged(int r);
	
private slots:

protected:
	virtual bool ReadXMLData();
	bool ReadRegValues();


private:
    Ui_SaisieEX ui;
};

#endif

