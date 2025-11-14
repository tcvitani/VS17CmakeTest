
#ifndef MFORM_LOGIN_H
#define MFORM_LOGIN_H

#include "MFormClientDlg.h"

class QLabel;
class QPushButton;
class QLineEdit;
class QFrame;

class MFormLogin: public MFormClientDlg
{
    Q_OBJECT

public:
    MFormLogin(QWidget *parent = 0);
	~MFormLogin();

	bool initialize(QString sLoginName, QString sTemplate, QString sEmbedTo, QString sCSSPath, bool bLoginFieldEnabled, bool bLDAPOffline);
	//if bLDAPOnline==true the sSecCodeis password otherwise it is the secure user id number
	void getLoginData(QString &sName, QString &sSecCode, bool &bLDAPOffline);
	
private:

	bool loadTemplate(QString sUIFilePath);
	void moveFocus(bool bUp);

	QLabel* m_lblTitle;
	QLabel* m_lblLoginName;
	QLabel* m_lblPassword;
	QLabel* m_lblMessage;
	QLineEdit* m_txtUserName;
	QLineEdit* m_txtPassword;
	QFrame *m_frContainer;
	bool m_bLDAPOffline;

};

#endif

