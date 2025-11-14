/****************************************************************************
**
** Copyright (C) 2005-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** Licensees holding a valid Qt License Agreement may use this file in
** accordance with the rights, responsibilities and obligations
** contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of this licensing
** agreement are not clear to you.
**
** Further information about Qt licensing is available at:
** http://www.trolltech.com/products/qt/licensing.html or by
** contacting info@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <MIhmMainLogic.h>
#include <MIhmWDThread.h>





int main(int argc, char *argv[])
{
	
	QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);

	QString sBlName;

	if(argc == 1) 	
		sBlName = QString("BL_IHM_PEAGER");
	else
		sBlName = argv[1];

	MIhmMainLogic::initTrace(sBlName);
	MIhmWDThread *pWDThread = new MIhmWDThread();  
	pWDThread->start();	

	MIhmMainLogic * core = new MIhmMainLogic();
	
	if(core->Initialize(sBlName) == INST_INIT_OK)
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

	MIhmMainLogic::deinitTrace();

	return 0;
}


