
#ifndef VIDEODLG_H
#define VIDEODLG_H

#include "Ui_videodlg.h"
#include "IHMSimuDld.h"


class VideoDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	VideoDlg(QWidget *parent, QString sIHMMailbox);

public slots:
	void accept();
	void reject();
	
private slots:
	void onCmdChanged(int);

protected:
	virtual bool ReadXMLData();
	bool ReadRegValues();

	
private:
    Ui_VideoDlg ui;
};

#endif

