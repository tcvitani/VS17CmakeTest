
#ifndef SAISIEDLG_H
#define SAISIEDLG_H

#include "Ui_SaisieDlg.h"
#include "IHMSimuDld.h"


class SaisieDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	SaisieDlg(QWidget *parent, QString sIHMMailbox);

public slots:
	void accept();
	void reject();
	void spinChanged(int r);
	
private slots:

protected:
	virtual bool ReadXMLData();
	bool ReadRegValues();


private:
    Ui_SaisieDlg ui;
};

#endif

