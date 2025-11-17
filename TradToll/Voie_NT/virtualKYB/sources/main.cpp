/****************************************************************************
**
** Copyright (C) 2014 Sanef Its Croatia.   All rights reserved.
****************************************************************************/

#include <QApplication>
#include <MKybMainLogic.h>
#include <MKybWDThread.h>

int main(int argc, char *argv[])
{
	
	QApplication app(argc, argv);
	
	QString sBlName, sMode;

	if(argc == 1) 	
		sBlName = QString("BL_VIRT_KYB");
	else if(argc == 2) 	
	{
		sBlName = argv[1];
	}
	if(argc == 3) 	//STANDALONE=0
	{
		sBlName = argv[1];
		sMode = argv[2];
	}

	MKybMainLogic::initTrace(sBlName);
	MKybWDThread *pWDThread = new MKybWDThread();  
	pWDThread->start();	

	MKybMainLogic * core = new MKybMainLogic();
	
	if(core->Initialize(sBlName, sMode) == INST_INIT_OK)
	{
		if(pWDThread->isRunning())
		{
			QObject::connect(pWDThread, SIGNAL(exitProgram()), core, SLOT(exitProgram()), Qt::QueuedConnection);
		}

		//Run the event loop 
		app.exec();
	}
	
	delete core;

	if(pWDThread->isRunning())
	{
		pWDThread->terminate();	
		pWDThread->wait(5000); //if nothing signaled wait max 5 secs
	}

	delete pWDThread;

	MKybMainLogic::deinitTrace();

	return 0;
}


