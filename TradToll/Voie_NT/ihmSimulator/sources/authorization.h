
#ifndef AUTHORIZATIONDLG_H
#define AUTHORIZATIONDLG_H

#include "Ui_authorization.h"
#include "IHMSimuDld.h"
#include "ihm.h"


class AuthorizationDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	AuthorizationDlg(QWidget *parent, int type, QString szMailbox);
	void setData(struct_ihm_authorization data);
	
public slots:
	void accept();
	void reject();
	
private slots:

protected:

	int m_nSubType;

	
private:
    Ui_authorization ui;
};

#endif

