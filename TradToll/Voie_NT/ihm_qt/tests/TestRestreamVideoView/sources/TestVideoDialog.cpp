#include <QtGui>
#include <QFileDialog>
#include <QFileInfo>
#include <QCoreApplication>

#include "TestVideoDialog.h"
#include "MRVideoView.h"
#include "MRVideoMsg.h"
#include "MRVideoCmdThread.h"
#include "MRVideoControlObject.h"
#include "windows.h"


TestVideoDialog::TestVideoDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

	connect(ui.btnSelectSrc,SIGNAL(clicked()),this,SLOT(onBtnSelectSrc()));
	connect(ui.btnHide,SIGNAL(clicked()),this,SLOT(onBtnHide()));
	connect(ui.btnShow,SIGNAL(clicked()),this,SLOT(onBtnShow()));

	connect(ui.btnDisconnectCtrl,SIGNAL(clicked()),this,SLOT(onBtnDisconnectCtrl()));
	connect(ui.btnConnectCtrl,SIGNAL(clicked()),this,SLOT(onBtnConnectCtrl()));
	
	connect(ui.btnGrab,SIGNAL(clicked()),this,SLOT(onBtnGrab()));
	connect(ui.btnSave,SIGNAL(clicked()),this,SLOT(onBtnSave()));
	connect(ui.btnZoom,SIGNAL(clicked()),this,SLOT(onBtnZoom()));
	connect(ui.btnClose,SIGNAL(clicked()),this,SLOT(onBtnClose()));
	connect(ui.btnStressTest,SIGNAL(clicked()),this,SLOT(onBtnStressTest()));

	connect(ui.btnSelectPath,SIGNAL(clicked()),this,SLOT(onBtnSelectPath()));
	connect(ui.btnOpen2,SIGNAL(clicked()),this,SLOT(onBtnOpen2()));
	

	m_pVideoView = NULL;
	pRVideoControlObject = NULL;
	initVideoView();
}

TestVideoDialog::~TestVideoDialog()
{
	delete m_pVideoView;

	delete pRVideoControlObject;
}

void TestVideoDialog::initVideoView()
{
	m_pVideoView = new MRVideoView(ui.frFrame);
	
	m_layout = new QHBoxLayout(ui.frFrame);
	m_layout->setSpacing(0);
	m_layout->setContentsMargins(0,0,0,0);	
	m_layout->addWidget(m_pVideoView);

	m_pVideoView->init();

	pRVideoControlObject = new MRVideoControlObject(0, "RVideo1");
	pRVideoControlObject->init("SOFTWARE\\CsRoute\\LaneController\\Config\\Modules\\IHM\\BL_IHM_PEAGER\\RestreamVideoSources");

	connect(pRVideoControlObject, SIGNAL(newOutputRVideoMessage(MRVideoMsg*, int)), this, SLOT(onRVideoResponseMessage(MRVideoMsg*, int)));
}

void TestVideoDialog::onRVideoResponseMessage(MRVideoMsg* pMsg, int iObjId)
{
	if(pMsg!=NULL)
	{
		ui.txtOutput->appendPlainText(QString("Received message:%1").arg(pMsg->getMessageDescription()));
	}
}



void TestVideoDialog::onBtnSelectSrc()
{
	pRVideoControlObject->activate(ui.spinSource->value());
	QString sFullStreamPath = pRVideoControlObject->getActivatedStreamSourceUrlLocal();

	ui.txtOutput->appendPlainText(QString("Selected source url:%1").arg(sFullStreamPath));
	m_pVideoView->SetSource(sFullStreamPath, "", "");


}

void TestVideoDialog::onBtnClose()
{
	close();
}

void TestVideoDialog::onBtnStressTest()
{

	for (int j= 0 ; j<10 ; j++)
	{
		for(int i= 0; i<=3;i++)
		{
			int tm = rand()%10;

			Sleep(tm);
			pRVideoControlObject->activate(i);
			m_pVideoView->SetSource(pRVideoControlObject->getActivatedStreamSourceUrlLocal(), "", "");
			onBtnShow();onBtnHide();onBtnShow();
			
			if(tm%2>0)
				QCoreApplication::processEvents();
		}
	}

}


void TestVideoDialog::onBtnShow()
{
	pRVideoControlObject->reactivate();
	m_pVideoView->ShowVideo();
}

void TestVideoDialog::onBtnHide()
{
	m_pVideoView->HideVideo();
	pRVideoControlObject->deactivate();
}


void TestVideoDialog::onBtnDisconnectCtrl()
{
	m_pVideoView->HideVideo();

}

void TestVideoDialog::onBtnConnectCtrl()
{
	m_pVideoView->ShowVideo();
}





void TestVideoDialog::onBtnGrab()
{
	pRVideoControlObject->grabb();
}

void TestVideoDialog::onBtnSave()
{
	bool bUseJPG = (ui.chkJPG->checkState()==Qt::Checked);
	
	pRVideoControlObject->save(ui.txtPath->text(), bUseJPG);
}

void TestVideoDialog::onBtnZoom()
{
	int iLeft,iTop,iWidth,iHeight;

	iLeft = ui.spinX->value();
	iTop = ui.spinY->value();
	iWidth = ui.spinWidth->value();
	iHeight = ui.spinHeight->value();


}



void TestVideoDialog::onBtnSelectPath()
{
	QFileInfo oFile(ui.txtPath->text());

	QString sPath = oFile.dir().path();
	sPath = QFileDialog::getSaveFileName(this, this->windowTitle(), sPath);

	bool bUseJPG = (ui.chkJPG->checkState()==Qt::Checked);
	QString sFileExt = bUseJPG?".JPG":".BMP";
	sPath.append(sFileExt);
	
	ui.txtPath->setText(sPath);
}

void TestVideoDialog::onBtnOpen2()
{
	TestVideoDialog * pNew = new TestVideoDialog(this);

	pNew->move(10,10);
	pNew->show();


}




