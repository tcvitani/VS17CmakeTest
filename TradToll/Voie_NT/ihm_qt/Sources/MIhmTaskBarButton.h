#ifndef MIHMTASKBARBUTTON_H
#define MIHMTASKBARBUTTON_H

#include <QPushButton>
//#include "MIhmTaskBarDlg.h"

//---------------------------------------
// MIhmTaskBarButton
//---------------------------------------
class MIhmTaskBarButton : public QPushButton
{
    Q_OBJECT
		
public:
	MIhmTaskBarButton(QWidget *parent, QString sName);
	~MIhmTaskBarButton();
	
	//void initialize(MIhmTaskBarDlg *dlg);
	int getType() {
		return 0;
	};//m_dlg->getType();};

	void makeMinimized();
	void makeDialogVisible();
	void makeClosed();
	bool openLink(QString sLink);

private slots:
	void onClicked();
	void onDlgClosed();
	void onDlgMinimize();
		
private:
//	MIhmTaskBarDlg * getDialog(){return m_dlg;};
//	MIhmTaskBarDlg *m_dlg;
	bool m_bClosed;
	
};


#endif

