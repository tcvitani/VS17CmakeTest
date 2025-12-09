/*************************************************************************\
*
*  PROGRAM: VMS WinTest
*  PURPOSE: To test VMS module.
*  COMMENTS:
*
\*************************************************************************/

#include <windows.h>
#include <time.h>
#include "resource1.h"
#include <noyau.h>
#include <tlm_dmv.h>
#include <wchar.h>

#define LOC_DEF
#include "test.h"
#undef LOC_DEF
#include "Test_ani.h"

#include <memclass.h>

#define	IDT_TIMER_DELAY		1

int  APIENTRY InitDlgProc (HWND, WORD, LONG, LONG);
void ExitApplication(HWND hDlg);
void AddLBItem(HWND hDlg, char *item);
enum_tlm_dmv_images StrToEnum(CHAR* ListItem);
BOOL StartPerformanceTest(HWND hDlg);
BOOL StopPerformanceTest(HWND hDlg);
void DoPerformanceTestStep(void);

HANDLE hHeap;
unsigned int index=0;
int tstex;
char szRandomNum[MAX_PATH] = {0};

int iNumStep;
CHAR gImages[4][50] =
{
	"TLM_DMV_IMAGE_FULL_GREEN", "TLM_DMV_IMAGE_FULL_RED",
	"TLM_DMV_IMAGE_GREEN_ARROW", "TLM_DMV_IMAGE_RED_CROSS"
};

PRIVATE struct_tache t_simu[] =
{
	{	TRUE,
		THREAD_PRIORITY_NORMAL,
		1024,
		(LPTHREAD_START_ROUTINE)(SimuRecoit),
		NULL,
		NULL,
		"SIMU" },

	{	FALSE,
		0,
		0,
		NULL,
		NULL,
		NULL,
		""}
};

// Creates the main dialogbox.
int APIENTRY WinMain (HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR     lpCmdLine,
                      int       nCmdShow)
{
  DWORD retCode;
  char *space;

  UNREFERENCED_PARAMETER( nCmdShow );
  UNREFERENCED_PARAMETER( lpCmdLine );
  UNREFERENCED_PARAMETER( hPrevInstance );

  SIMU.hInst   = hInstance;
  hHeap   = HeapCreate (0, 0, 0);

  if (strcmp(lpCmdLine, ""))
	  strcpy_s(SIMU.szBalName, sizeof(SIMU.szBalName), lpCmdLine);
  else
  {
	  MessageBoxA(NULL, "You have to enter module bal name and \".dll\" name in command line! Letters are no case sensitive. Ex. BL_XYZ dll_name.dll",
		  "WARNING", MB_OK | MB_ICONWARNING);
	  return 0;
  }

  // convert letters to upper
  for (int i = 0; i < strlen(lpCmdLine); i++)
	  lpCmdLine[i] = toupper(lpCmdLine[i]);

  // check if exist 'space' in command line
  space = strchr(lpCmdLine, ' ');

  if (space == NULL)
  {
	  //if 'space' no exist, check is it "*.dll"
	  if (strcmp(&lpCmdLine[strlen(lpCmdLine) - 3], "DL"))
	  {
		  MessageBoxA(NULL, "You have to enter \"*.dll\" name in command line! Letters are no case sensitive",
			  "WARNING", MB_OK | MB_ICONWARNING);
		  return 0;
	  }
	  else
		  strcpy_s(SIMU.szDllName, sizeof(SIMU.szDllName), lpCmdLine);
  }
  else
  {
	  //if 'space' exist take first part for SIMU.szBalName and second for DllName
	  strcpy_s(SIMU.szBalName, sizeof(SIMU.szBalName), lpCmdLine);
	  SIMU.szBalName[strlen(SIMU.szBalName) - strlen(space)] = '\0';
	  strcpy_s(SIMU.szDllName, sizeof(SIMU.szDllName), &space[1]);

	  // check is it really "*.dll"
	  if (strcmp(&SIMU.szDllName[strlen(SIMU.szDllName)-3], "DL"))
	  {
		  MessageBoxA(NULL, "You have to enter \"*.DLL\" name in command line!",
			  "WARNING", MB_OK | MB_ICONWARNING);
		  return 0;
	  }
  }

  retCode = (DWORD)DialogBox((HANDLE)SIMU.hInst, (LPCWSTR)IDD_MAIN, NULL, (DLGPROC)InitDlgProc);

  HeapDestroy (hHeap);

  return  (retCode);
}

// PURPOSE:  Handle the menu command
int APIENTRY InitDlgProc (HWND hDlg, WORD wMsg, LONG wParam, LONG lParam)
{
	UNREFERENCED_PARAMETER( lParam );
	switch (wMsg)
	{
		case WM_INITDIALOG:
			if(LanceModule(hDlg)==1)
			{
				char szTemp[MAX_PATH + 1] = { 0 };
				sprintf_s(szTemp, sizeof(szTemp), "Module %s with bal name %s started.", SIMU.szDllName, SIMU.szBalName);
				AddLBItem(hDlg, szTemp);
				//	memset(&SIMU.message, 0, sizeof(SIMU.message));
				SendDlgItemMessage(hDlg, IDC_LIST1, LB_SETHORIZONTALEXTENT, 3000, 0);
			}
			else
				EndDialog (hDlg, TRUE);
			break;

		case WM_SIZE :
			// Resizing the list box
			MoveWindow( GetDlgItem(hDlg, IDC_LIST1), 
						3, 
						1, 
						LOWORD(lParam)-6, 
						HIWORD(lParam)-4, 
						TRUE);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				// ETAT SERVICE
				case ID_ETAT_START:
					Envoi(M_SRV_ETAT, SRV_TYP_DEBUT, &SIMU.message);
					break;
				
				case ID_ETAT_REQUEST:
					Envoi(M_SRV_ETAT, SRV_TYP_DEMANDE, &SIMU.message);
					break;

				case ID_ETAT_STOP:
					Envoi(M_SRV_ETAT, SRV_TYP_FIN, &SIMU.message);
					break;

				// ESPION SERVICE
				case ID_ESPION_START:
					DialogBox(SIMU.hInst, (LPCTSTR)IDD_ESPION, hDlg, (DLGPROC)StartEspion);
					break;

				case ID_ESPION_STOP:
					Envoi(M_SRV_ESPION, SRV_TYP_FIN, &SIMU.message);
					break;
				
				// ARRET SERVICE
				case ID_ARRET_REQUEST:
					Envoi(M_SRV_ARRET, SRV_TYP_DEMANDE, &SIMU.message);
					break;

				// VMS SERVICE
				case ID_MESSENGERSERVICE_START:
					Envoi(M_TLM_MESSENGER_SERVICE, SRV_TYP_DEBUT, &SIMU.message);
					break;

				case ID_MESSENGERSERVICE_STOP:
					Envoi(M_TLM_MESSENGER_SERVICE, SRV_TYP_FIN, &SIMU.message);
					break;

				case ID_MESSENGERSERVICE_SETIMAGE:
					DialogBox(SIMU.hInst, (LPCTSTR)IDD_SET_SYMBOL, hDlg, (DLGPROC)SetImageRequest);
					Envoi(M_TLM_MESSENGER_SERVICE, SRV_TYP_SET, &SIMU.message);
					break;
				
				case ID_MESSENGERSERVICE_CYCLICAIMAGESET:
					DialogBox(SIMU.hInst, (LPCTSTR)IDD_CYCLIC_SET_SYMBOL, hDlg, (DLGPROC)CyclicSetImageRequest);
					break;
					
				// ABOUT BOX
				case IDD_ABOUT:
					DialogBox(SIMU.hInst, (LPCTSTR)IDD_ABOUT, hDlg, (DLGPROC)About);
					break;

				// Exit from aplication
				case ID_FILE_EXIT:
					ExitApplication(hDlg);
					break;
		
				case IDCANCEL:
					ExitApplication(hDlg);
					break;
			}
	}
	return (FALSE);
}


// Add item into ListBox.
void AddLBItem(HWND hDlg, char *item)
{
	SendDlgItemMessage( hDlg, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM) (LPSTR) item ) ;
	index=(unsigned int)SendDlgItemMessage( hDlg, IDC_LIST1, LB_GETCOUNT , 0, 0 ) ;
	SendDlgItemMessage( hDlg, IDC_LIST1, LB_SETCURSEL , index-1, 0 ) ;
}

// Process starting of ESPION  service
LRESULT CALLBACK StartEspion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			CheckRadioButton(hDlg,IDC_RADIO_DONNES,IDC_RADIO_SERVICE,IDC_RADIO_SERVICE);
			CheckRadioButton(hDlg,IDC_RADIO_ENTRANT,IDC_RADIO_E_S,IDC_RADIO_E_S);
			SIMU.dop_nature = IDC_RADIO_SERVICE;
			SIMU.dop_sens = IDC_RADIO_E_S;
			return TRUE;

		case WM_COMMAND:
			if (HIWORD(wParam)==BN_CLICKED)
			{
				if ((LOWORD(wParam)>=IDC_RADIO_DONNES)
					&&(LOWORD(wParam)<=IDC_RADIO_SERVICE))
				{
					SIMU.dop_nature=LOWORD(wParam);
					CheckRadioButton(hDlg,IDC_RADIO_DONNES,IDC_RADIO_SERVICE,LOWORD(wParam));
				}
				if ((LOWORD(wParam)>=IDC_RADIO_ENTRANT)
					&&(LOWORD(wParam)<=IDC_RADIO_E_S))
				{
					SIMU.dop_sens=LOWORD(wParam);
					CheckRadioButton(hDlg,IDC_RADIO_ENTRANT,IDC_RADIO_E_S,LOWORD(wParam));
				}
			}
			switch(LOWORD(wParam))
			{

				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;

				case IDOK:
					switch(SIMU.dop_nature)
					{
						case IDC_RADIO_DONNES:
							SIMU.message.srv.espion.nature = SRV_ESPION_MESSAGE_DONNEES;
							break;

						case IDC_RADIO_SERVICE:
							SIMU.message.srv.espion.nature = SRV_ESPION_MESSAGE_SERVICE;
							break;
					}
					switch(SIMU.dop_sens)
					{
						case IDC_RADIO_ENTRANT:
							SIMU.message.srv.espion.sens = SRV_ESPION_ENTRANT;
							break;

						case IDC_RADIO_SORTANT:
							SIMU.message.srv.espion.sens = SRV_ESPION_SORTANT;
							break;

						case IDC_RADIO_E_S:
							SIMU.message.srv.espion.sens = SRV_ESPION_ENTRANT_SORTANT;
							break;
					}
					Envoi(M_SRV_ESPION, SRV_TYP_DEBUT, &SIMU.message);
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
					break;
			}
			return TRUE;
	}
    return FALSE;
}
// Process trigger request
LRESULT CALLBACK SetImageRequest(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char szVal[MAX_PATH + 1] = {0};
	int  i = 0;
	CHAR Images[4][50] =
	{
		"TLM_DMV_IMAGE_FULL_GREEN", "TLM_DMV_IMAGE_FULL_RED",
		"TLM_DMV_IMAGE_GREEN_ARROW", "TLM_DMV_IMAGE_RED_CROSS"
	};
	CHAR A[1024];
	static CHAR CurrentListItem[MAX_PATH];

	memset(&A, 0, sizeof(A));

	switch (message)
	{
		case WM_INITDIALOG:
			// Setting combobox
			for (i = 0; i <= 5; i++) // 6 = number of symbols in TCHAR Symbols
			{
				strcpy_s(A, sizeof(A), Images[i]);
				SendDlgItemMessage(hDlg, IDC_COMBO_SYMBOLS, CB_ADDSTRING, 0, (LPARAM)A);
			}

			SendDlgItemMessage(hDlg, CB_SETCURSEL, (WPARAM)1, 0, (LPARAM)0);
			return TRUE;

		case WM_COMMAND:
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				__int64 ItemIndex = SendDlgItemMessage(hDlg, IDC_COMBO_SYMBOLS, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				(CHAR)SendDlgItemMessage(hDlg, IDC_COMBO_SYMBOLS, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)CurrentListItem);
			}

			if (LOWORD(wParam) == IDOK)
			{
				SIMU.message.srv.srv_tlm.u.display_srv.state = StrToEnum(CurrentListItem);
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}

			if (LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			
			break;
			return TRUE;
	}
    return FALSE;
}

LRESULT CALLBACK CyclicSetImageRequest(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:
		EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE); // Disable STOP button
		SetDlgItemInt(hDlg, IDC_DELAY, 1000, FALSE); // set delay to 1000 ms
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_START:
			if (StartPerformanceTest(hDlg) == TRUE)
			{
				//disable START button and enable STOP button
				EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
			}
			break;
		case IDC_STOP:
			if (StopPerformanceTest(hDlg) == TRUE)
			{
				//enable START button and disable STOP button
				EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
			}
			break;
		case IDC_EXIT:
			if (SIMU.bCyclicStarted)
			{
				if (StopPerformanceTest(hDlg) == TRUE)
				{
					//enable START button and disable STOP button
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
				}
			}
			EndDialog(hDlg, TRUE);
			break;
		}
		break;
	case WM_CLOSE:
		if (SIMU.bCyclicStarted)
		{
			if (StopPerformanceTest(hDlg) == TRUE)
			{
				//enable START button and disable STOP button
				EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
			}
		}
		EndDialog(hDlg, TRUE);
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case IDT_TIMER_DELAY:
			iNumStep++;
			DoPerformanceTestStep();
			break;
		}
		break;
		return TRUE;
	}
	return FALSE;
}

BOOL StartPerformanceTest(HWND hDlg)
{
	unsigned char		szTemp[MAX_PATH] = { 0 };
	UINT				uiRes = 0;
	UINT				uElapse = 0;

	// get timer delay, if less than 100 ms abort with warning
	uElapse = GetDlgItemInt(hDlg, IDC_DELAY, NULL, FALSE);
	if (uElapse < 100)
	{
		MessageBoxA(hDlg, "Cycle Delay minimum value is 100 ms!", "Warning", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	// start timer
	if (!SetTimer(hDlg, IDT_TIMER_DELAY, uElapse, NULL))
	{
		MessageBoxA(hDlg, "SetTimer() function failed!", "Warning", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;

	}

	SIMU.bCyclicStarted = TRUE;

	return TRUE;
}

BOOL StopPerformanceTest(HWND hDlg)
{
	unsigned char		szTemp[MAX_PATH] = { 0 };
	UINT				uiIdx = 0;
	UINT				uElapse = 0;
	struct_tlm_message *p_message = &SIMU.message;

	Envoi(M_TLM_MESSENGER_SERVICE, SRV_TYP_FIN, &SIMU.message);
	SIMU.bCyclicStarted = FALSE;

	// Stop timer
	if (!KillTimer(hDlg, IDT_TIMER_DELAY))
	{
		MessageBoxA(hDlg, "KillTimer() function failed!", "Warning", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	return TRUE;
}

void DoPerformanceTestStep(void)
{
	if (iNumStep > 5)
		iNumStep = 0;

	SIMU.message.srv.srv_tlm.u.display_srv.state = StrToEnum(gImages[iNumStep]);
	Envoi(M_TLM_MESSENGER_SERVICE, SRV_TYP_SET, &SIMU.message);
}

enum_tlm_dmv_images StrToEnum(CHAR* ListItem)
{
	if (strcmp(ListItem, "TLM_DMV_IMAGE_FULL_GREEN") == 0)
		return TLM_DMV_IMAGE_FULL_GREEN;
	else if (strcmp(ListItem, TEXT("TLM_DMV_IMAGE_FULL_RED")) == 0)
		return TLM_DMV_IMAGE_FULL_RED;
	else if (strcmp(ListItem, TEXT("TLM_DMV_IMAGE_GREEN_ARROW")) == 0)
		return TLM_DMV_IMAGE_GREEN_ARROW;
	else if (strcmp(ListItem, TEXT("TLM_DMV_IMAGE_RED_CROSS")) == 0)
		return TLM_DMV_IMAGE_RED_CROSS;
	else
		return TLM_DMV_IMAGE_FULL_RED;
}

// Process about DialogBox 
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}

// Exit aplication.
void ExitApplication(HWND hDlg)
{
	int tstex;
	tstex= MessageBoxA( NULL, "Stop tasks and exit aplication?",
		"Confirmation", MB_OKCANCEL | MB_ICONQUESTION | 
		MB_SYSTEMMODAL );
	if (tstex==IDOK)
	{
		SIMU.fin=TRUE;
		ArretModule(hDlg);
		EndDialog (hDlg, TRUE);
	}
}

// Send message into MailBox.
void Envoi(short int service, short int type_message, struct_tlm_message *p_message)
{
	struct_tlm_message  *p_msg_emis;

	ExitAlloue((struct_neutre **)(&p_msg_emis),
		sizeof(struct_tlm_message),NOYAU_GetPoolId("SIMU_TEST"));

	p_msg_emis->entete.service = service;
	p_msg_emis->entete.type_message = type_message;
	switch( service )
	{
		case M_SRV_ESPION:
			memcpy( &p_msg_emis->srv.espion, &p_message->srv.espion,
                    sizeof(p_message->srv.espion));
			break;

		case M_TLM_MESSENGER_SERVICE:
			switch (p_msg_emis->entete.type_message)
			{
			case SRV_TYP_DEBUT:
				break;
			case SRV_TYP_FIN:
				break;
			
			case SRV_TYP_SET:
				memcpy(&p_msg_emis->srv.srv_tlm.u.display_srv, &p_message->srv.srv_tlm.u.display_srv, sizeof(p_message->srv.srv_tlm.u.display_srv));
				break;
			}
			break;

		case M_SRV_ARRET:
		case M_SRV_ETAT:
		default:
			break;
	}
	ExitEnvoie(SIMU.module_bal, SIMU.test_bal, (struct_neutre *)(p_msg_emis));
}

// Start module.
int LanceModule(HWND hDlg)
{
	char                   szKey[MAX_PATH + 1] = {0};
	enum_instance_result   rc;
	char                   temp[50] = {0};

	SIMU.inst_id++;
	sprintf_s(szKey, sizeof(szKey), "%s%s%s%s%s%s",
			CSR_REG_KEYn_CSRBASE,
			CSR_REG_KEYn_LANE_BASE,
			CSR_REG_KEYn_CONFIG,
			MOD_REG_KEYn_MODULES,
			"VMS\\",
            SIMU.szBalName );

	SIMU.hDLL = LoadLibrary(SIMU.szDllName);

	if ( SIMU.hDLL != NULL)
	{
		SIMU.LpFncLance = (TLM_LANCE)GetProcAddress(SIMU.hDLL, FNC_LANCE);
		SIMU.LpFncArret = (TLM_ARRET)GetProcAddress(SIMU.hDLL, FNC_ARRET);

		if ((!SIMU.LpFncLance) || (!SIMU.LpFncArret))
		{
			FreeLibrary(SIMU.hDLL);
			sprintf_s(temp, sizeof(temp), TEXT("Cannot find function \"%s\" or \"%s\" in \"%s\""), FNC_LANCE, FNC_ARRET, SIMU.szDllName);
			MessageBoxA(NULL, temp, "ERROR", MB_OK | MB_ICONWARNING);
			return 0;
		}
	}
	else
	{
		sprintf_s(temp, sizeof(temp), TEXT("Cannot load dll : \"%s\""), SIMU.szDllName);
		MessageBoxA(NULL, temp, "ERROR", MB_OK | MB_ICONERROR);
		return 0;
	}

	rc = SIMU.LpFncLance(szKey, SIMU.szBalName, &SIMU.module_bal);
	if (rc != INST_INIT_OK)
	{
		sprintf_s(temp, sizeof(temp), TEXT("Probleme VMSLance => %d \n"), rc);
		MessageBoxA(NULL, temp, "ERROR", MB_OK | MB_ICONERROR);
		FreeLibrary(SIMU.hDLL);
		return 0;
	}

	SIMU.module_bal = AttendBAL( SIMU.szBalName );
	if ( SIMU.module_bal <= 0 )
	{
		sprintf_s(temp, sizeof(temp), TEXT("Probleme AttendBAL(\"%s\")\n"), "WARNING");
		MessageBoxA(hDlg, temp, "WARNING", MB_OK|MB_ICONWARNING);
		return 0;
	}

	DebutRegion() ;
	SIMU.test_bal = PublieBAL ("SIMU_TEST", NOYAU_BAL_ILLIMITEE);
	SIMU.test_bal = AttendBAL ("SIMU_TEST");
	FinRegion();

	rc = LanceTache (t_simu);
	SIMU.hDlg=hDlg;
	if ( rc != NOYAU_OK)
	{
		SIMU.LpFncArret( SIMU.module_bal );
		FreeLibrary(SIMU.hDLL);
		return 0;
	}

	return 1;
}

// Stop MIF1 module.
int ArretModule(HWND hDlg)
{
	enum_instance_result   rc;
	char                   temp[50] = {0};

	rc = SIMU.LpFncArret(SIMU.module_bal);
	if ( rc != INST_INIT_OK)
	{
		sprintf_s(temp, sizeof(temp), "Probleme MODArret => %d ", rc);
		AddLBItem(hDlg, temp);
		return 0;
	}
	else
	{
		AddLBItem(hDlg, "Module arrete");
	}

	// Stop thread
	rc = ArretTaches( t_simu );
	if ( rc != NOYAU_ARRET_TACHE_OK)
	{
		AddLBItem(hDlg, "Error stoping thread");
		return 0;
	}
	else
		AddLBItem(hDlg, "Simulator stopped");

	SupprimeBAL ("SIMU_TEST");

	return 1;
}