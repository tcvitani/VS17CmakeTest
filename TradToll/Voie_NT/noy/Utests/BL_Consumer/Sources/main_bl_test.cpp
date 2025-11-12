
#include "noyau.h"
#include <conio.h>

//#pragma comment( lib, "..\\debug\\CSR_NOY.LIB" )

#define NUM_THREADS_TO_USE 10

DWORD WINAPI Thread (PVOID param)
{
    noyau_bal_id bal_rec;
    struct_neutre *pp_neutre;
	struct_neutre *pp_neutre_sent;
	DWORD dwResult;
	char szMailboxName[MAX_PATH];	
	noyau_taille_bloc dwSize;

	strcpy_s(szMailboxName, sizeof(szMailboxName), (char*)param);

    bal_rec = PublieBAL (szMailboxName, 0); 

 	printf("Published mailbox:%s id:%d",szMailboxName, (int)bal_rec);

    while(1)
    {
        dwResult = Recoit(bal_rec, (struct_neutre **) &pp_neutre, NOYAU_ATTENTE_INFINIE);
		
		if(dwResult==NOYAU_BAL_MESS)
		{
			dwSize = DonneTailleBloc(pp_neutre);
			ExitAlloue (&pp_neutre_sent, dwSize, 0);
			memcpy(pp_neutre_sent, pp_neutre, dwSize);
			//return the message...
			ExitEnvoie (pp_neutre->bl_retour, bal_rec, pp_neutre_sent);

			ExitLibere(&pp_neutre);
 		}
		else
		{
			printf("Consumer Thread exit with noyau code %d",dwResult);
			ExitBad();
		}
    }

    return 0;
}

void main (int argc, char *argv[ ])
{
    noyau_bal_id bal_rec;
    struct_neutre *pp_neutre;
	struct_neutre *pp_neutre_sent;
	DWORD dwResult;
	char szMailboxName[MAX_PATH];	
	char szMailboxNames[10][MAX_PATH];	
	noyau_bal_id aBal_cons[10];
	noyau_taille_bloc dwSize;

	DWORD ThreadId[NUM_THREADS_TO_USE];		
	int i;

	if(argc == 2)
	{
		strcpy_s(szMailboxName, sizeof(szMailboxName), argv[1]);
	}
	else
	{
		strcpy_s(szMailboxName, sizeof(szMailboxName), "BL_REC1");
	}
	
	sprintf_s(szMailboxNames[0], sizeof(szMailboxNames[0]), "%s_%d", szMailboxName, 0);
	bal_rec = PublieBAL (szMailboxNames[0], 0); 

	printf("Published mailbox:%s id:%d",szMailboxNames[0], (int)bal_rec);


	for(i=1;i<NUM_THREADS_TO_USE;i++)
	{
		sprintf_s(szMailboxNames[i], sizeof(szMailboxNames[i]), "%s_%d", szMailboxName, i);
	    CreateThread (NULL, 0, Thread, szMailboxNames[i], 0, &ThreadId[i]);    
	}

	for(i=1;i<NUM_THREADS_TO_USE;i++)
	{
		aBal_cons[i] = AttendBALTantQue (szMailboxNames[i],100);

		if(aBal_cons[i] == NOYAU_NOK)
		{
			printf("AttendBALTantQue failure %s",szMailboxNames[i]);
		}
	}


	
    while(1)
    {
        dwResult = Recoit(bal_rec, (struct_neutre **) &pp_neutre, NOYAU_ATTENTE_INFINIE);
		
		
		if(dwResult==NOYAU_BAL_MESS)
		{
			dwSize = DonneTailleBloc(pp_neutre);
			ExitAlloue (&pp_neutre_sent, dwSize, 0);
			memcpy(pp_neutre_sent, pp_neutre, dwSize);
			//return the message...
			ExitEnvoie (pp_neutre->bl_retour, pp_neutre->bl_id, pp_neutre_sent);

			ExitLibere(&pp_neutre);
 		}
		else
		{
			printf("Consumer main thread exit with noyau code %d",dwResult);
			ExitBad();
		}
    }

	for(i=1;i<NUM_THREADS_TO_USE;i++)
	{
	    TerminateThread ((HANDLE)ThreadId[i], dwResult);    
	}

	_getch();
    return;
}