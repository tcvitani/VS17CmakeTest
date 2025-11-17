


#include "MKybWDThread.h"
#include "MTracer.h"

extern "C" {
	#include <virt_kyb.H>
	#include <lwd.h>
};


MKybWDThread::MKybWDThread()
{

}


MKybWDThread::~MKybWDThread()
{
	quit();
    wait();
}


void MKybWDThread::run()
{

	HANDLE hStop ;
    DWORD dwErr;

    TRACE_D("MKybWDThread::run:..." );

	msleep(1000);
	// Recuperer un handle sur l'evenement
	hStop = OpenEventA (EVENT_MODIFY_STATE|SYNCHRONIZE, FALSE, LWD_EVT_APPLICATION_STOP) ;

	// Si on a un acces sur l'evenement ...
	if (hStop != NULL)
    {
		// Attente ...
		WaitForSingleObject (hStop, INFINITE) ;
		
		TRACE_W("MKybWDThread::LWD_EVT_APPLICATION_STOP was signaled! Initiating exit program..." );
		emit exitProgram();
    }
	else
	{
        dwErr = GetLastError();

        TRACE_W(QString("MKybWDThread::run: OpenEvent(%1) retourn %2")
									.arg(LWD_EVT_APPLICATION_STOP)
									.arg(dwErr));
	}

    TRACE_D("MKybWDThread::EXIT!" );

}

