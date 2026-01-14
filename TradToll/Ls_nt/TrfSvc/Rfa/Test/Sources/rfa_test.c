#include <windows.h>
#include <stdio.h>
#include <csr_rfa.h>
#include <trc.h>

#include <io.h>
#include <ImageHlp.h>


typedef struct
{
	char record_size[4];
	char record_type[2];
	char separator_1;
	char account_id[6];
	char separator_2;
	char account_count[6];
} struct_record_header;
/*--------------------------- FUNCTIONS: --------------------------*/
/*--------------------------- VARIABLES: --------------------------*/


typedef enum
{
	RFA_NO_ERROR = 0,
	RFA_ERR_OPEN_FILE,
	RFA_ERR_READ_FILE,
	RFA_ERR_SET_PTR,
	RFA_ERR_CLOSE_FILE
}enum_err_white_list;


TRC_EMETTEUR ghTrc;
DWORD gdwTraceMask;

#define BUF_SIZE 255


//-------------------------------------------------------------------
// A function to Display the message indicating in which tread we are
//-------------------------------------------------------------------
void TraceMessage(char *sMessage)
{
	char msgBuf[BUF_SIZE];

	sprintf_s(msgBuf, sizeof(msgBuf), "PID:%d THREAD:%d: %s", GetCurrentProcessId(), GetCurrentThreadId(), sMessage);
	TRC_Trace_Texte(ghTrc, gdwTraceMask, msgBuf);

}

long StrToLong(IN char *field, IN unsigned char length)
{
	char buffer[10];

	memset(buffer, 0, length + 1);
	memcpy(buffer, field, length);
	return (atol(buffer));
}

enum_err_white_list FindAccount(IN char *tag,
	OUT char *status)
{
	long account_id, tag_account, ptr_position;
	DWORD rc, nb_read;
	struct_record_header str_rec_head;
	HANDLE hFile;
	DWORD err = NO_ERROR;

	// file key from the registry.
	// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\TrfSvc\Parameters\Files\
			//"WHITELIST" ="EXPORT(.)=c:\\temp\\WHITELIST.DAT"

	hFile = RFACreateFile("WHITELIST", 0, 0, 0, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		TraceMessage("ERROR RFACreateFile");
		return RFA_ERR_OPEN_FILE;
	}

	RFAEnter(hFile);
		TraceMessage("RFAEnter1");
		Sleep(1);


	// get the record header
	if (!RFAReadFile(hFile, &str_rec_head, sizeof(str_rec_head), &nb_read, NULL))
	{
		err = ERROR_FILE_NOT_FOUND;
	}

	if (err == NO_ERROR)
	{
		account_id = StrToLong((char *)&str_rec_head.account_id, sizeof(str_rec_head.account_id));
		tag_account = StrToLong(&tag[1], 5);// account number
		ptr_position = tag_account - account_id;

		if (ptr_position < 0)
		{
			err = INVALID_SET_FILE_POINTER;
		}
	}



	if (err == NO_ERROR)
		err = RFASetFilePointer(hFile, ptr_position, NULL, FILE_CURRENT);

	if (err == NO_ERROR)
		if (!RFAReadFile(hFile, status, 1, &nb_read, NULL))
		{
			err = RFA_ERR_READ_FILE;
		}

	TraceMessage("RFALeave1");
	Sleep(1);
	RFALeave(hFile);

	if (!RFACloseHandle(hFile))
		return RFA_ERR_CLOSE_FILE;

	return RFA_NO_ERROR;
}



DWORD WINAPI Thread_FindAccount(LPVOID lpParam)
{
	enum_err_white_list result;
	char tag[100];
	char status[100];
	int i = 1;
	int iCount = 0;
	HANDLE  hStdout = NULL;
	HANDLE hMutex = NULL;

	TraceMessage("Created Thread!");

	while (1)
	{
		i++;

		memset(tag, 0, sizeof(tag));
		memset(status, 0, sizeof(status));
		tag[0] = ' ';
		sprintf_s(&tag[1], sizeof(tag)-1, "%d", i);

		result = FindAccount(tag, status);

		if (RFA_NO_ERROR != result)
		{
			sprintf_s(status, sizeof(status), "\nAccount:%s, Result:%d", tag, result);
			TraceMessage(status);
		}
		else
		{
			if (i % 100 == 0)
			{
				sprintf_s(status, sizeof(status), "Verified Account:%s", tag);
				TraceMessage(status);
			}
		}


		Sleep(10);

		if (i > 10000)
		{
			sprintf_s(status, sizeof(status), "Verified 10000 accounts");
			TraceMessage(status);
			i = 1;

			iCount++;

			//if(iCount>10)
			//	RFAMoveFileEx("C:\\CSR\\DATA\\TWL\\$WHITE$.$$$", "WhiteList", MOVEFILE_REPLACE_EXISTING);

		}
	}

	TraceMessage("Thread exit");


	Sleep(1000);
	return 0;
}


BOOL initTrace()
{
	DWORD dwErr = NO_ERROR;
	char path[MAX_PATH];

	/* creation du repertoire de traces */
	if (_access("C:\\CSR\\TRACES\\", 0) != 0)
		if (!MakeSureDirectoryPathExists("C:\\CSR\\TRACES\\"))
			return FALSE;

	/* creation du chemin complet vers le fichier de traces avec ajout de l'extension par defaut */
	sprintf_s(path, sizeof(path), "%s\\%s.TRC", "C:\\CSR\\TRACES\\", "RFA_TEST");

	dwErr = TRC_Initialise_Trace("RFA_TEST", path, TRC_OPT_FICHIER | TRC_OPT_CONSOLE, &ghTrc);
	gdwTraceMask = TRC_OPT_FICHIER;

	TRC_Taille_Max_Fichier(ghTrc, 10000000);

	return TRUE;

}



#define NUM_THREADS 20

int main(int argc, char ** argv)
{
	int k;
	HANDLE Array_Of_Thread_Handles[NUM_THREADS];   // Aray to store thread handles 

	initTrace();

	printf("Start FindAccount test:\n");

	//RFAMoveFileEx("C:\\CSR\\DATA\\TWL\\$WHITE$.$$$", "WhiteList", MOVEFILE_REPLACE_EXISTING);

	for (k = 0; k < NUM_THREADS; k++)
	{
		// Create thread 1.
		Array_Of_Thread_Handles[k] = CreateThread(NULL, 0, Thread_FindAccount, NULL, 0, NULL);

		if (Array_Of_Thread_Handles[k] == NULL)
			ExitProcess(0);
	}


	// Wait until all threads have terminated.
	WaitForMultipleObjects(NUM_THREADS, Array_Of_Thread_Handles, TRUE, INFINITE);


	TraceMessage("Thread Exit!");

	printf("Since All threads executed lets close their handles \n");

	// Close all thread handles upon completion.
	for (k = 0; k < NUM_THREADS; k++)
	{
		CloseHandle(Array_Of_Thread_Handles[k]);
	}

	printf("Finished!");
}

