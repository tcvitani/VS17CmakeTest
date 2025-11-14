#ifndef MIHMTASKBARDLG_H
#define MIHMTASKBARDLG_H

#include "windows.h"
#include <QString>
#include <QTimer>
#include <QDialog>

//#include "shdocvw.h"

class QVBoxLayout;
class QFrame;
class QLabel;
class QPushButton;
class MFormMain;

class SHDocVw::WebBrowser;


struct MMemMSGStruct
{
	MSG oMsg;
	bool bFiltered;
	char cGeneratedChar;
};

//---------------------------------------
// MIhmTaskBarDlg
//---------------------------------------
// web browser dialog
class MIhmTaskBarDlg : public QDialog
{
	Q_OBJECT
		
public:
	MIhmTaskBarDlg(MFormMain *pMainDlg, int nType, QString sTitle);
	~MIhmTaskBarDlg();
	bool initialize(QString sUITemplate, const QRect &rectGeometry = QRect());
	bool openLink(QString sLink);
	int getType(){return m_nType;};

	static MFormMain *m_pMainDlg;	
	static MMemMSGStruct m_oOldMSG;	

	void setTitleText(QString sTitle);
	void loadDefaultPage();
signals:
	void closed();
	void minimized();
		
private slots:
	void onTimeout();
	
private:

	bool event(QEvent * event); 

	bool installNewHandler();
	bool uninstallNewHandler();
	
	QWidget *m_dynFormWidget;
	QVBoxLayout *m_layout;
	QFrame *m_frContainer;
	QFrame *m_frIEContainer;
	
	//SHDocVw::WebBrowser * m_pIEControl;
	QTimer tm;
	
	HWND m_iHandleToIE;
	
	int m_iIERetries;
	
	int m_nType;
	QString m_sTitle;

};


#endif