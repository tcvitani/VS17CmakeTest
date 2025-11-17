
#ifndef MAINDLG_H
#define MAINDLG_H

#include "ui_MainDlg.h"
#include "ihm.h"

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
	MainDlg(QWidget *parent, QString szMailbox);
	void executeScript(QString szScript, bool bPrepend);

public slots:
    void onSet_Config();
    void onGet_Config();
    void onDebutAff();
    void onSet_Icon();
    void onSet_Label();
    void onSet_Image();
	void onVideo();
    void onFinAff();
    void onDebutTouche();
    void onDemandeTouche();
    void onFinTouche();
    void onDemandeSasie();
    void onAnnulationSasie();
    void onDemandeSasieEx();
    void onAnnulationSasieEx();
    void onDebutDetection();
    void onFinDetection();
    void onSetPolice();
    void onAbout_IHM_Simulator();
    void onSetText();
    void onArret();
	void onWaitExit();
	void onScript();
	void onLoopScriptChange();
	void onScriptAction();
	void onMessage(QString);
	void onLink();
	void onAsyncInputDebut();
	void onAsyncInputSet();
	void onAsyncInputGet();
	void onAsyncInputFin();
	
	void onAuthDebut();
	void onAuthFin();
	void onAuthAutoChecked();
	void onAuthTakeCtrlAutoChecked();
	void onAuthReturnCtrlAutoChecked();
	void onAuthRsp();
	void onAuthTakeCtrlRsp();
	void onAuthReturnCtrlRsp();
	void onAuthData(struct_ihm_authorization*);
	void onClose();	

	void onWaitUserReq();

signals:
	void prependScript(QString)	;
	void appendScript(QString)	;
	
private slots:
    void on_aboutQtButton_clicked();
protected:
	virtual void closeEvent(QCloseEvent * event);
	void OpenXMLScript(QString szScript);

private:
    Ui_MainDlg				ui;

	QString					m_sSystemEncoding;

	ConfigDlg				*m_pConfDlg;
	AffichageDlg			*m_pAffDlg;
	VideoDlg				*m_pVideoDlg;
	ToucheDlg				*m_pToucheDlg;
	SaisieDlg				*m_pSaisieDlg;
	SaisieExDlg				*m_pSaisieExDlg;
	PoliceDlg				*m_pPoliceDlg;
	DetectionDlg			*m_pDetectionDlg;
	AuthorizationDlg		*m_pAuthDlg;
	LinkDlg					*m_pLinkDlg;
	AsyncInputDlg			*m_pAsyncInputDlg;

	SimuSendReciveThread	*m_pSendReciveThread;



private:
	bool						m_bExit;
	struct_ihm_authorization	m_AuthData;
	QString m_sIHMMailbox;
};

#endif

