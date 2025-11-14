
#ifndef ASYNCINPUTDLG_H
#define ASYNCINPUTDLG_H

#include "Ui_AsyncInputDlg.h"
#include "IHMSimuDld.h"


class AsyncInputDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	AsyncInputDlg(QWidget *parent, QString sIHMMailbox);

public slots:
	void accept();
	void reject();
	
private:
    Ui_AsyncInput ui;
};

#endif

