

#include <QtGui>
#include <QtTest/QtTest>
#include "TestBAL_Process.h"
#include "Test_helper.h"


int doTestBal_processes(int iSize, noyau_bal_id gbal_rec, noyau_bal_id gbal_emi);

unsigned char * pRandomData = NULL;
const char gszMainProducerMailbox[] = "BL_PRODUCER_MAIN";

DWORD WINAPI ThreadProducer(PVOID param)
{
    noyau_bal_id bal_my;
    struct_neutre *pp_neutre;
	struct_neutre *pp_neutre_sent;

	DWORD dwResult;
	char szMailboxName[MAX_PATH];	
	int i,j,iSize;
	char sMbName[MAX_PATH];
	char sMsg[MAX_PATH];
	noyau_bal_id m_aBal_cons[NUM_PROCESSES_TO_TEST][NUM_THREADS_TO_USE];
	
	noyau_bal_id m_aBal_main_test;

	strcpy_s(szMailboxName, sizeof(szMailboxName), (char*)param);

    bal_my = PublieBAL (szMailboxName, 0); 
	
	m_aBal_main_test = AttendBALTantQue((char*)gszMainProducerMailbox,100);
	
	if(m_aBal_main_test == NOYAU_NOK)
	{
		sprintf_s(sMsg, sizeof(sMsg), "AttendBALTantQue failure %s", sMbName);
		QTest::qFail(sMsg, __FILE__, __LINE__);
		return -1;
	}

	//wait for mailboxes of the created processes
	for(i=0;i<NUM_PROCESSES_TO_TEST;i++)
	{
		for(j=0;j<NUM_THREADS_TO_USE;j++)
		{
			sprintf_s(sMbName, sizeof(sMbName), "BL_CONSUMER%d_%d", i, j);
			m_aBal_cons[i][j] = AttendBALTantQue(sMbName,100);

			if(m_aBal_cons[i][j] == NOYAU_NOK)
			{
				sprintf_s(sMsg, sizeof(sMsg), "AttendBALTantQue failure %s", sMbName);
				QTest::qFail(sMsg, __FILE__, __LINE__);
				return -1;
			}
		}
	}


    while(1)
    {
        dwResult = Recoit(bal_my, (struct_neutre **) &pp_neutre, NOYAU_ATTENTE_INFINIE);
		
		
		if(dwResult==NOYAU_BAL_MESS)
		{	
			if(m_aBal_main_test == pp_neutre->bl_retour)
			{
				iSize = DonneTailleBloc(pp_neutre);

				for(i=0;i<NUM_PROCESSES_TO_TEST;i++)
				{
					for(j=0;j<NUM_THREADS_TO_USE;j++)
					{
						ExitAlloue (&pp_neutre_sent, iSize, 0);
						memcpy(pp_neutre_sent, pp_neutre, iSize);
						
						ExitEnvoie (m_aBal_cons[i][j], bal_my, pp_neutre_sent);
					}
				}

				memset(pp_neutre,0,iSize);
				ExitLibere (&pp_neutre);

			}
			else
			{
				dwResult = Envoie (m_aBal_main_test, bal_my, pp_neutre);
				
				if(dwResult == NOYAU_NOK)
				{
					sprintf_s(sMsg, sizeof(sMsg), "Envoie failed for mbox %d", m_aBal_cons[i][j]);
					QTest::qFail(sMsg, __FILE__, __LINE__);
					return -1;
				}
			}

 		}
		else
		{
			printf("ThreadProducer exit with noyau code %d",dwResult);
			break;
		}
    }

    return 0;
}


void TestBAL_Process::initTestCase()
{
	int i,j;
	
	QString s = QString("Number processes %1, Num threads: %2").arg(NUM_PROCESSES_TO_TEST).arg(NUM_THREADS_TO_USE);
	QWARN(s.toLatin1().data());



	//start a list of consumer processes
	for(i=0;i<NUM_PROCESSES_TO_TEST;i++)
	{
		MyProcess * newProcess = new MyProcess();
		lstConsumerProcess.append(newProcess);
		
		QString sMbName = QString("BL_CONSUMER%1").arg(i);
		newProcess->m_sMailboxName = sMbName;
	}


	MyProcess * currentProcess;

	for(i=0;i<NUM_PROCESSES_TO_TEST;i++)
	{
		currentProcess = lstConsumerProcess.at(i);	
	
#ifdef _DEBUG
		currentProcess->start("BL_CONSUMER.exe", QStringList(currentProcess->m_sMailboxName));
#else
		currentProcess->start("BL_CONSUMER_R.exe", QStringList(currentProcess->m_sMailboxName));
#endif	
	}



// 	for(i=0;i<NUM_PROCESSES_TO_TEST;i++)
// 	{
// 		currentProcess = lstConsumerProcess.at(i);	
// 
// 		QProcess::ProcessState e = currentProcess->state();
// 		if(e==QProcess::Starting || e==QProcess::Running)
// 		{
// 			currentProcess->waitForStarted(3000);
// 		}
// 		else
// 		{
// 			QFAIL(QString("Unable to start process BLConsumer.exe %1").arg(currentProcess->m_sMailboxName).toAscii().data());
// 		}
// 	}	
// 
	m_szMainMailboxName = QString(gszMainProducerMailbox);
	m_mainMailboxId = PublieBAL ((char*)gszMainProducerMailbox, 0); 
	 
	//create my mailbox
	//create producer threads and publish mailboxes
	for(i=0;i<NUM_THREADS_TO_USE;i++)
	{
		strcpy_s(m_szProdMailboxNames[i], sizeof(m_szProdMailboxNames[i]), QString("BL_PRODUCER_%1").arg(i).toLatin1().data());
	    CreateThread (NULL, 0, ThreadProducer, m_szProdMailboxNames[i], 0, &ThreadId[i]);    
	}

	//wait for mailboxes of the created processes
	for(i=0;i<NUM_PROCESSES_TO_TEST;i++)
	{
		currentProcess = lstConsumerProcess.at(i);	
		
		for(j=0;j<NUM_THREADS_TO_USE;j++)
		{
			QString sMbName = QString("BL_CONSUMER%1_%2").arg(i).arg(j);
			currentProcess->m_aBal_cons[j] = AttendBALTantQue(sMbName.toLatin1().data(),100);

			if(currentProcess->m_aBal_cons[j] == NOYAU_NOK)
			{
				QFAIL(QString("AttendBALTantQue failure %1").arg(sMbName).toLatin1().data());
			}
		}
	}
	



	for(i=0;i<NUM_THREADS_TO_USE;i++)
	{
		m_aBal_prod[i] = AttendBALTantQue (m_szProdMailboxNames[i],100);

		if(m_aBal_prod[i] == NOYAU_NOK)
		{
			QFAIL(QString("AttendBALTantQue failure %1").arg(m_szProdMailboxNames[i]).toLatin1().data());
		}
	}

	pRandomData = (unsigned char*)malloc(1048576);
	FillRandomData(pRandomData, 1048576);

}

void TestBAL_Process::cleanupTestCase()
{
	int i;
	QString szMailboxNameTemp;

	for(i=0;i<NUM_THREADS_TO_USE;i++)
	{
		SupprimeBAL(m_szProdMailboxNames[i]);
	}
	
	MyProcess * pProcess;

	while (!lstConsumerProcess.isEmpty())
	{
		pProcess = lstConsumerProcess.takeFirst();
		pProcess->kill();

		pProcess->waitForFinished();
		delete pProcess;
	}

	free(pRandomData);

}

void TestBAL_Process::testREMOTE_BALs_data()
{
	QTest::addColumn<int>("Size");
	QTest::addColumn<int>("Result");

	QTest::newRow("1") << 1 << (int)NOYAU_BAL_MESS;
	QTest::newRow("1000") << 1000 << (int)NOYAU_BAL_MESS;

//	QTest::newRow("2000") << 2000 << (int)NOYAU_BAL_MESS;

//	QTest::newRow("3000") << 3000 << (int)NOYAU_BAL_MESS;

//	QTest::newRow("10000") << 10000 << (int)NOYAU_BAL_MESS;

//	QTest::newRow("20000") << 20000 << (int)NOYAU_BAL_MESS;

// 	QTest::newRow("50000") << 50000 << (int)NOYAU_BAL_MESS;
// 
// 	QTest::newRow("60000") << 60000 << (int)NOYAU_BAL_MESS;

// 	QTest::newRow("100000") << 100000 << (int)NOYAU_BAL_MESS;
// 
// 	QTest::newRow("500000") << 500000 << (int)NOYAU_BAL_MESS;

// 	QTest::newRow("1048576") << 1048576 << (int)NOYAU_BAL_MESS;
// 
// 	QTest::newRow("2048576") << 2048576 << (int)NOYAU_BAL_MESS;
// 	
// 	QTest::newRow("3048576") << 3048576 << (int)NOYAU_BAL_MESS;
	
//	QTest::newRow("5048576") << 5048576 << (int)NOYAU_BAL_MESS;

// 	QTest::newRow("10048576") << 10048576 << (int)NOYAU_BAL_MESS;
// 	QTest::newRow("20048576") << 20048576 << (int)NOYAU_BAL_MESS;
// 	QTest::newRow("40048576") << 40048576 << (int)NOYAU_BAL_MESS;

}

void TestBAL_Process::testREMOTE_BALs()
{

	QFETCH(int, Size);
	QFETCH(int, Result);
	
	QCOMPARE(doTestBal_remote_process(Size), Result);
	QBENCHMARK{doTestBal_remote_process(Size);}
}


//-----------------------------------------------------------

int TestBAL_Process::doTestBal_remote_process(int iSize)
{
	DWORD dwResult;
	BYTE pOriginalMessage[100000];
	struct_neutre *pp_neutre_sent;  
	struct_neutre *pp_neutre_received;  
	int iSize2;
	bool bSizeOk;
	bool bMemoryOK;
	bool bLastByteOK;

	iSize = iSize + sizeof(struct_neutre);
	SetRandomData(pOriginalMessage, iSize, pRandomData);


	for(int k=0;k<NUM_MESSAGES_PER_PRODUCER;k++)
	{
		for(int j=0;j<NUM_THREADS_TO_USE;j++)
		{

			ExitAlloue (&pp_neutre_sent, iSize, 0);
			memcpy(pp_neutre_sent, pOriginalMessage, iSize);

			ExitEnvoie (m_aBal_prod[j], m_mainMailboxId, pp_neutre_sent);

			int i=0;

			while(i<NUM_PROCESSES_TO_TEST*NUM_THREADS_TO_USE )
			{	
				dwResult = Recoit (m_mainMailboxId, &pp_neutre_received, NOYAU_ATTENTE_INFINIE);

				if(dwResult == NOYAU_BAL_MESS)
				{
					i++;
					iSize2 = DonneTailleBloc(pp_neutre_received);

					bSizeOk = (iSize == iSize2);
					bMemoryOK = (memcmp((byte*)pOriginalMessage+sizeof(struct_neutre), 
								(byte*)pp_neutre_received+sizeof(struct_neutre), 
								iSize - sizeof(struct_neutre))==0);
					bLastByteOK = (*((byte*)pOriginalMessage+iSize-1) == *((byte*)pp_neutre_received+iSize-1));

					if(!bSizeOk || !bMemoryOK || !bLastByteOK)
					{
						printf("doTestBal_processes: Compared data does not match! from mailbox:%d\n", pp_neutre_received->bl_retour);
						printf("bSizeOk:%d, bMemoryOK:%d, bLastByteOK:%d , size:%d\n", bSizeOk, bMemoryOK, bLastByteOK, iSize - (int)sizeof(struct_neutre));

						QByteArray baSent((const char*)((byte*)pOriginalMessage+sizeof(struct_neutre)), iSize - sizeof(struct_neutre));
						QByteArray baReceived((const char*)((byte*)pp_neutre_received+sizeof(struct_neutre)), iSize - sizeof(struct_neutre));

						if(!bMemoryOK)
						{
							int m;
							
							for (m = 0; m < iSize - sizeof(struct_neutre); m++)
							{
								if(baSent.at(m)!=baReceived.at(m))
								{
									break;
								}
							}
							
							if(m<iSize - sizeof(struct_neutre))
							{
								printf("First byte different at pos:%d\n", m);
								printf("Sent: %s\n", baSent.mid(m,100).toHex().data());
								printf("Received: %s\n", baReceived.mid(m,100).toHex().data());
							
							}
							else
									printf("WARRNING: No difference found!\n", m);
						}

						dwResult = NOYAU_NOK;
						ExitLibere (&pp_neutre_received);
						break;
					}
					//else OK
					memset(pp_neutre_received,0,iSize);
					ExitLibere (&pp_neutre_received);

				}
				else
				{
					printf("doTestBal_remote_process: Recoit return error:%d!\n", dwResult);
					
					break;
				}
			}

		}
	}

	return dwResult;
}



