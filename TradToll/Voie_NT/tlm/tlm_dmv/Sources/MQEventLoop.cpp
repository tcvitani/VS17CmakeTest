/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 MQEventLoop													*/
/* FILE:		MQEventLoop.cpp															 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

extern "C"
{
#include <noyau.h>

}


#include <MQEventLoop.h>
#include <QCoreApplication>

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/
#define MAX_INSTANCES 100

/*-------------------------------- TYPEDEFS:  -------------------------------*/
class MQEventLoop
{
public:
	QEventLoop * pLoop;
	HANDLE evtLoopFinished;
	LPVOID pvWorkerStructure;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

static MQEventLoop GLOB_LOOP[MAX_INSTANCES] = { 0 };
static int giInstanceCounter = 0;

/*---------------------------------- CODE: ----------------------------------*/
EXPORT_C int MQEventLoop_initQtLoop()
{
	int iNewInstance = -1;

	DebutRegionLocale();
	if (giInstanceCounter < MAX_INSTANCES)
		iNewInstance = giInstanceCounter++;
	FinRegionLocale();

	return iNewInstance;
}

int MQEventLoop_exit(int iLoopHandle)
{
	if (iLoopHandle >= 0)
	{
		QEventLoop* pEventLoop = GLOB_LOOP[iLoopHandle].pLoop;
		if (pEventLoop != NULL)
			pEventLoop->exit(0);

		return 0;
	}

	return 1;
}

EXPORT_C void MQEventLoop_Quit(int iLoopHandle)
{
	DWORD dwWaitResult;
	BOOL isError;

	isError = MQEventLoop_exit(iLoopHandle);

	if (!isError)
	{
		dwWaitResult = WaitForSingleObject(GLOB_LOOP[iLoopHandle].evtLoopFinished, 30000);

		CloseHandle(GLOB_LOOP[iLoopHandle].evtLoopFinished);
	}
}

EXPORT_C DWORD WINAPI MQEventLoop_LoopThread(PVOID pvParam)
{
	TWorkerStructure *psWorkerStructure = (TWorkerStructure *)pvParam;

	int iLoopHandle = MQEventLoop_initQtLoop();


	if (iLoopHandle >= 0)
	{
		QEventLoop* pEventLoop = new QEventLoop();
		GLOB_LOOP[iLoopHandle].pLoop = pEventLoop;

		GLOB_LOOP[iLoopHandle].evtLoopFinished = CreateEvent(
			NULL,               // default security attributes
			FALSE,               // auto-reset event
			FALSE,              // initial state is non-signaled
			TEXT("evtLoopFinished")  // object name
			);

		GLOB_LOOP[iLoopHandle].pvWorkerStructure = (PVOID*)psWorkerStructure;

		//Initialize the event driven logic
		psWorkerStructure->m_iLoopInstance = iLoopHandle;
		psWorkerStructure->pfInitWorker(psWorkerStructure);

		pEventLoop->exec();

		//Uninitialize the event driven logic...
		psWorkerStructure->pfDeinitWorker(psWorkerStructure);

		delete pEventLoop;
		GLOB_LOOP[iLoopHandle].pLoop = NULL;

		SetEvent(GLOB_LOOP[iLoopHandle].evtLoopFinished);
	}

	return FALSE;
}

