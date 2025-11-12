#ifndef NOY_TEST_BAL_PROCESS_H
#define NOY_TEST_BAL_PROCESS_H

#include <QObject.h>
#include <QProcess.h>
#include "noyau.h"

#define NUM_THREADS_TO_USE 10
#define NUM_PROCESSES_TO_TEST 8
#define NUM_MESSAGES_PER_PRODUCER 100

class MyProcess : public QProcess
{
public:
	MyProcess():QProcess(){};
	~MyProcess(){};
		
	QString m_sMailboxName;
	noyau_bal_id m_aBal_cons[NUM_THREADS_TO_USE];
};


class TestBAL_Process : public QObject
{
    Q_OBJECT
private slots:
	void initTestCase();

	void testREMOTE_BALs_data();
	void testREMOTE_BALs();


	void cleanupTestCase();

private:	
	int doTestBal_remote_process(int iSize);


	QList <MyProcess *> lstConsumerProcess;

	noyau_bal_id m_aBal_prod[NUM_THREADS_TO_USE];
	noyau_bal_id m_mainMailboxId;

	QString m_szMainMailboxName;
	char m_szProdMailboxNames[NUM_THREADS_TO_USE][MAX_PATH];
	DWORD ThreadId[NUM_THREADS_TO_USE];		
	int m_iNumProcessesToTest;
};

#endif