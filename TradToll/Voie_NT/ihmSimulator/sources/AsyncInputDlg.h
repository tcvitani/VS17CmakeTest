
#ifndef ASYNCINPUTDLG_H
#define ASYNCINPUTDLG_H

#include "IHMSimuDld.h"

class Ui_AsyncInput;

class AsyncInputDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	AsyncInputDlg(QWidget *parent, QString sIHMMailbox);

public slots:
	void accept();
	void reject();
	
private:
    Ui_AsyncInput *ui;
};

#endif

