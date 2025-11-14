#ifndef MIHMTASKBAR_H
#define MIHMTASKBAR_H

#include <QList>
#include <QFrame>
#include <QHBoxLayout>

class MIhmTaskBarButton;
class MFormMain;



//---------------------------------------
// MIhmTaskBar
//---------------------------------------
// taskbar class
class MIhmTaskBar : public QFrame
{
    Q_OBJECT

public:
    MIhmTaskBar(QFrame *parent);
	~MIhmTaskBar();
	
	void init(MFormMain *pMainWindow, QString sDlgUITemplatePath);
	void initDialog(int nType, QString sName,  const QRect &rectGeometry);
	void openBrowserDlg(int nType, QString sLink);
	void closeBrowserDlg(int nType);

	void minimizeAll();
	void closeAll();

protected:
	
	QList<MIhmTaskBarButton*> m_lstDlgButtons;
    QHBoxLayout *m_horizontalLayout;
	MFormMain *m_pParentWindow;

	MIhmTaskBarButton * findButton(int nType);
	void addNewDialog(int nType, QString sName, const QRect &rectGeometry);
	
	QString m_sDlgUITemplatePath;


};

#endif

