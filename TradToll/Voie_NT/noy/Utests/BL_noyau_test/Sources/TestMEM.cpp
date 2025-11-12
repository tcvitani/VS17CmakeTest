

#include <QtGui>
#include <QtTest/QtTest>
#include "TestMEM.h"
#include "Test_helper.h"

#define NUM_THREADS_TO_USE 10
#define NUM_MESSAGES_PER_THREADS 10
#define NUM_PROCESSES_TO_TEST_MEM 10


void TestMEM::initTestCase()
{
	int i;
	
	QString s = QString("Memory test Num processes:%1").arg(NUM_PROCESSES_TO_TEST_MEM);
	QWARN(s.toLatin1().data());

	//start a list of consumer processes
	for(i=0;i<NUM_PROCESSES_TO_TEST_MEM;i++)
	{
		QProcess * newProcess = new QProcess();
		lstConsumerProcess.append(newProcess);
	}

	QProcess * currentProcess;

	for(i=0;i<NUM_PROCESSES_TO_TEST_MEM;i++)
	{
		currentProcess = lstConsumerProcess.at(i);	
	
#ifdef _DEBUG
		currentProcess->start("TEST_MEM.exe");
#else
		currentProcess->start("TEST_MEM_R.exe");
#endif	
	}


	for(i=0;i<NUM_PROCESSES_TO_TEST_MEM;i++)
	{
		currentProcess = lstConsumerProcess.at(i);	

		QProcess::ProcessState e = currentProcess->state();
		if(e==QProcess::Starting || e==QProcess::Running)
		{
			currentProcess->waitForStarted(3000);
			
			
		}
		else
		{
			QFAIL(QString("Unable to start process TEST_MEM ").toLatin1().data());
		}
	}	
}

void TestMEM::cleanupTestCase()
{
	QProcess * pProcess;

	while (!lstConsumerProcess.isEmpty())
	{
		pProcess = lstConsumerProcess.takeFirst();
		pProcess->kill();
		delete pProcess;
	}

}


void TestMEM::testMemAlloc()
{

	QCOMPARE(doTestMEM(), (DWORD)0);
}


//-----------------------------------------------------------

DWORD TestMEM::doTestMEM()
{
	DWORD dwResult = 0;
	int i;
	DWORD	dwCount=0;
	QProcess *currentProcess;

	while(1)
	{
		for(i=0;i<NUM_PROCESSES_TO_TEST_MEM;i++)
		{
			currentProcess = lstConsumerProcess.at(i);	

			QProcess::ProcessState e = currentProcess->state();
			
			if(e==QProcess::NotRunning)
			{
				QWARN(QString("Process num:%1 has stopped").arg(i).toLatin1().data());
				dwResult = 1;
				break;
			}
		}		

		Sleep(10);

		dwCount++;

		if(dwCount>1000)
			break;
	}


	return dwResult;
}



