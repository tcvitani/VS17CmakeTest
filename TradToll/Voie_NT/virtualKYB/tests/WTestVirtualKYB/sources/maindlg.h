
#ifndef MAINDLG_H
#define MAINDLG_H

#include "ui_MainDlg.h"
#include "virt_kyb.h"

class ConfigDlg;
class AffichageDlg;
class AuthorizationDlg;
class VideoDlg;
class ToucheDlg;
class SaisieDlg;
class SaisieExDlg;
class PoliceDlg;
class DetectionDlg;
class LinkDlg;
class AsyncInputDlg;
class SimuSendReciveThread;


class MainDlg : public QMainWindow
{
    Q_OBJECT

public:
    MainDlg(QString sBlName, QWidget *parent = 0);
	void executeScript(QString szScript, bool bPrepend);

public slots:
	void onSendAffMove();
	void onSendAffShow();
	void onSendAffHide();
	void onSendInitConfig();
	void onSendAffDEBUT();
	void onSendAffFIN();
	void onSendArret();
	void onMessage(QString);
	void onClose();	

signals:
	void prependScript(QString)	;
	void appendScript(QString)	;
	
protected:
	virtual void closeEvent(QCloseEvent * event);
	void OpenXMLScript(QString szScript);

private:
    Ui_MainDlg				ui;

	QString					m_sSystemEncoding;

	AffichageDlg			*m_pAffDlg;
	SimuSendReciveThread	*m_pSendReciveThread;

private:
	bool						m_bExit;
};

#endif

