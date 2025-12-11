/***************** (v) 2016 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	  QT Module QApplication integration							 */
/* FILE:	MQApplication.cpp												 */
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
#include <csrlc32.h>
#include <reg.h>
#include <noyau.h>
#include <debug.h>
#include <run.h>
}



#include <QCoreApplication>
#include <QtLogging>

#include "MQApplication.h"
#include "InstTracer.h"

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/
static 	HANDLE appFinished = NULL;
static QCoreApplication * app = NULL;
static int qcoreOwnerInstId = NULL;
static int numQtInstancesPerProces;
static int argc = 1;
static const char * argv[] = { "SharedLibrary", NULL };

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

/*---------------------------------- CODE: ----------------------------------*/
static void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	QString sMsg;

	if (qcoreOwnerInstId != NULL)
	{
		switch (type)
		{
		case QtDebugMsg:
			MInstTracer_FichierTrace(0, (char*)"QtDebugMsg: %s", msg.toLatin1().data());
			break;
		case QtWarningMsg:
			MInstTracer_FichierTrace(0, (char*)"QtWarningMsg: %s", msg.toLatin1().data());
			break;
		case QtFatalMsg:
			MInstTrtacer_FichierError(0, (char*)"QtFatalMsg: %s", msg.toLatin1().data());
			abort();                       // dump core on purpose
		}
	}
}

DWORD WINAPI ApplicationThread(PVOID pvParam)
{
	Q_UNUSED(pvParam);

	if (QCoreApplication::instance() == NULL)
	{
		qInstallMessageHandler(myMessageOutput);
		app = new QCoreApplication(argc, (char**)argv);

		app->exec();

		SetEvent(appFinished);
	}


	return 0;
}


DWORD WINAPI StartApplicationThread()
{
	//struct_tache sThreads[1 + 1];
	char pcNomTache[MAX_PATH] = { 0 };
	noyau_enum_retour eResult;
	HANDLE hThread = NULL;
	DWORD dwThreadId = NULL;
	DWORD dwErr = NO_ERROR;

	appFinished = CreateEvent(
		NULL,               // default security attributes
		FALSE,               // auto-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("appFinished")  // object name
		);

	sprintf_s(pcNomTache, sizeof(pcNomTache), "QApplication thread ");

	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(ApplicationThread), (void*)NULL, CREATE_SUSPENDED, &dwThreadId);
	if (hThread == NULL)
	{
		dwErr = GetLastError();
		ExitBad();
		return dwErr;
	}

	if (ResumeThread(hThread) == -1)
	{
		dwErr = GetLastError();
		ExitBad();
		return dwErr;
	}

	return dwErr;
}


DWORD WINAPI StopApplicationThread()
{
	DWORD dwWaitResult;

	if (app != NULL)
	{
		app->exit(0);

		if (appFinished != NULL)
		{
			dwWaitResult = WaitForSingleObject(appFinished, 10000);
			CloseHandle(appFinished);
			appFinished = NULL;
		}

		qInstallMessageHandler(nullptr);

		delete app;

		return 0;
	}

	return 1;
}
