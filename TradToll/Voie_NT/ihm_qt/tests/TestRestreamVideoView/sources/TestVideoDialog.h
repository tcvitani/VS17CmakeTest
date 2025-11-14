
#ifndef TestVideoDialog_H
#define TestVideoDialog_H

#include "ui_TestVideoDialog.h"

class QHBoxLayout;
class MRVideoView;
class MRVideoMsg;
class MRVideoCmdThread;
class MRVideoControlObject;

class TestVideoDialog: public QDialog
{
    Q_OBJECT

public:
    TestVideoDialog(QWidget *parent = 0);
	~TestVideoDialog();


private slots:
	void onBtnSelectSrc();

	void onBtnShow();
	void onBtnHide();
	void onBtnClose();
	void onBtnGrab();
	void onBtnSave();
	void onBtnZoom();
	void onBtnSelectPath();
	void onBtnOpen2();
	void onBtnStressTest();
	void onBtnDisconnectCtrl();
	void onBtnConnectCtrl();
	
	void onRVideoResponseMessage(MRVideoMsg* pMsg, int iObjId);

private:

	void initVideoView();
	Ui_TestVideoDialog ui;
	
	MRVideoView * m_pVideoView;
	QHBoxLayout *m_layout;

	MRVideoControlObject * pRVideoControlObject;

};

#endif

