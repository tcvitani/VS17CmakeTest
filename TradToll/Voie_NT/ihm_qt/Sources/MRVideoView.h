
#ifndef MRVIDEO_VIEW_H
#define MRVIDEO_VIEW_H

#include <QWidget>
#include <QFrame>
#include "Cs_MJPG_Ctrl.h"


class QHBoxLayout;
class QVBoxLayout;
class QFrame;
class QLabel;




class MRVideoView: public QFrame
{
    Q_OBJECT

public:
    MRVideoView(QWidget *parent = 0);
	~MRVideoView();

	bool init();
	bool SetSource(QString sURL, QString sUser, QString sPwd);
	bool ShowVideo();
	bool HideVideo();
	bool isInitialized(){return m_bInitialized;};
private slots:
	void onCtlException(int,QString,QString,QString);

private:
	bool reInitAxis();

	QWidget *m_parent;
	QHBoxLayout *m_layout;
    CS_MJPG_CONTROLLib::CS_MjpgCtrl *m_CamCtl;

	QString m_sRegVideoSrcs;
	QString m_sURL;
	QString m_sUser;
	QString m_sPwd;

	bool m_bInitialized;
};




#endif

