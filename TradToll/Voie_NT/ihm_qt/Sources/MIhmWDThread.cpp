


#include "MIhmWDThread.h"
#include "MTracer.h"

extern "C" {
	#include <ihm.H>
	#include <lwd.h>
};


MIhmWDThread::MIhmWDThread()
{

}


MIhmWDThread::~MIhmWDThread()
{
	quit();
    wait();
}


void MIhmWDThread::run()
{

	HANDLE hStop ;
    DWORD dwErr;

    TRACE_D("MIhmWDThread::run:..." );

	msleep(1000);
	// Recuperer un handle sur l'evenement
	hStop = OpenEventA (EVENT_MODIFY_STATE|SYNCHRONIZE, FALSE, LWD_EVT_APPLICATION_STOP) ;

	// Si on a un acces sur l'evenement ...
	if (hStop != NULL)
    {
		// Attente ...
		WaitForSingleObject (hStop, INFINITE) ;
		
		TRACE_W("MIhmWDThread::LWD_EVT_APPLICATION_STOP was signaled! Initiating exit program..." );
		emit exitProgram();
    }
	else
	{
        dwErr = GetLastError();

        TRACE_W(QString("MIhmWDThread::run: OpenEvent(%1) retourn %2")
									.arg(LWD_EVT_APPLICATION_STOP)
									.arg(dwErr));
	}

    TRACE_D("MIhmWDThread::EXIT!" );

}

