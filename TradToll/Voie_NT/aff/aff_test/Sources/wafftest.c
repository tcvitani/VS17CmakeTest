/*************************************************************************\
*
*  PROGRAM: wafftest
*  PURPOSE: To test AFF module.
*  COMMENTS:
*
\*************************************************************************/

#include <windows.h>
#include "wafftest.h"
#include "waff.h"
#include "simu.h"
#include "resource.h"
#include <aff_ext.h>

#include <memclass.h>

#define	AFF_MAX_PRIORITY	7
#define	IDT_TIMER_DELAY		1

void ExitApplication(HWND hDlg);
void DisableMenu(HWND hDlg);
void EnableMenu(HWND hDlg);
void AddLBItem(HWND hDlg, char *item);
LRESULT CALLBACK Reset(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Display(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TestDisplay(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Start_Espion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BrowseDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PerformanceTest(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL StartPerformanceTest(HWND hDlg);
BOOL StopPerformanceTest(HWND hDlg);
void DoStop(HWND hDlg);
void DoPerformanceTestStep(void);



CHAR			MailBoxName[MAX_PATH];
HANDLE			hInst;
HWND			hDlg;
HANDLE			hHeap;
int				index=0;
char szTmp[MAX_PATH]="";


/*************************************************************************\
*
*  FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
*
*  PURPOSE: Creates the dialogbox.
*
*  COMMENTS:
*
\*************************************************************************/

int APIENTRY WinMain (HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR     lpCmdLine,
                      int       nCmdShow)
{
	DWORD	retCode;
	char	*space;
	
	UNREFERENCED_PARAMETER( nCmdShow );
	UNREFERENCED_PARAMETER( lpCmdLine );
	UNREFERENCED_PARAMETER( hPrevInstance );
	
	hInst   = hInstance;
	hHeap   = HeapCreate (0, 0, 0);

	// default mailbox name
	strcpy_s(MailBoxName, sizeof(MailBoxName), "BL_AFF");

	// check if exist 'space' in command line
	space = strchr(lpCmdLine, ' ');

	if (space == NULL)
	{	
		// if 'space' no exist, check is it "*.dll"
		if (strcmp (&lpCmdLine[strlen(lpCmdLine)-3], "dll"))
		{
			MessageBox(NULL, "You have to enter \"*.dll\" name in command line!",
						"WARNING", MB_OK | MB_ICONWARNING);
			return 0;
		}
		else
			strcpy_s(dll_name, sizeof(dll_name), lpCmdLine);
	}
	else
	{
		// if 'space' exist take first part for MailBoxName, and second for Dll_name
		strcpy_s(MailBoxName, sizeof(MailBoxName), lpCmdLine);
		MailBoxName[strlen(MailBoxName) - strlen(space)] = '\0';
		strcpy_s(dll_name, sizeof(dll_name), &space[1]);

		// check is it really "*.dll"
		if (strcmp (&dll_name[strlen(dll_name)-3], "dll"))
		{
			MessageBox(NULL, "You have to enter \"*.dll\" name in command line!", "WARNING", MB_OK | MB_ICONWARNING);
			return 0;
		}
	}
			
 	retCode = (int)DialogBox ((HANDLE)hInst, (LPCSTR)"TestAff", NULL, (DLGPROC)InitDlgProc);
	
	HeapDestroy (hHeap);

	return  (retCode);
}

/************************************************************************\
*
*  FUNCTION: InitDlgProc();
*
*  PURPOSE:  Handle the AFF module messages.
*
\************************************************************************/

int APIENTRY InitDlgProc (HWND hDlg, WORD wMsg, LONG wParam, LONG lParam)
{

	struct_aff_message *p_message = &SIMU.message;
	UNREFERENCED_PARAMETER( lParam );

	switch (wMsg)
    {
    case WM_INITDIALOG:
		// AFF module initialization
		if(LanceModule(hDlg, MailBoxName)==1)
			AddLBItem(hDlg, "AFF module started");
		else
			EndDialog (hDlg, TRUE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
        {
			// Processing menu options. Menu options sends
			// messages into AFF module MailBox
		case IDM_AFFICHAGE_DEBUT:
			simu_envoi_aff (SIMU.aff_bal, M_AFF_AFFICHAGE, SRV_TYP_DEBUT);
			break;

		case IDM_AFFICHAGE_DISPLAY:
			DialogBox(hInst, (LPCTSTR)DISPLAY, hDlg, (DLGPROC)Display);
			
			break;


		case IDM_AFFICHAGE_RESET:
			DialogBox(hInst, (LPCTSTR)RESET, hDlg, (DLGPROC)Reset);
			break;

		case IDM_AFFICHAGE_FIN:
			simu_envoi_aff (SIMU.aff_bal, M_AFF_AFFICHAGE, SRV_TYP_FIN);
			break;

		case IDM_AFFICHAGE_BACKLIGHT_ON:
			p_message->u.srv_affichage.allume.etat = TRUE;
			simu_envoi_aff (SIMU.aff_bal, M_AFF_AFFICHAGE, AFF_TYP_ALLUME);
			break;

		case IDM_AFFICHAGE_BACKLIGHT_OFF:
			p_message->u.srv_affichage.allume.etat = FALSE;
			simu_envoi_aff (SIMU.aff_bal, M_AFF_AFFICHAGE, AFF_TYP_ALLUME);

			break;

		case IDM_AFFICHAGE_NEW_FILE:
			DialogBox(hInst, (LPCTSTR)IDD_BROWSE, hDlg, (DLGPROC)BrowseDlgProc);
			break;

		case IDM_AFFICHAGE_PERFO_TEST:
			DialogBox(hInst, (LPCTSTR)IDD_PERFORMANCE_DIALOG, hDlg, (DLGPROC)PerformanceTest);
			break;

		case IDM_ESPION_START:
			DialogBox(hInst, (LPCTSTR)START_ESPION, hDlg, (DLGPROC)Start_Espion);
			break;

		case IDM_ESPION_END:
			simu_envoi_aff (SIMU.aff_bal, M_SRV_ESPION, SRV_TYP_FIN);
			break;

		case IDM_ETAT_DEBUT:
			simu_envoi_aff (SIMU.aff_bal, M_SRV_ETAT, SRV_TYP_DEBUT);
			break;

		case IDM_ETAT_FIN:
			simu_envoi_aff (SIMU.aff_bal, M_SRV_ETAT, SRV_TYP_FIN);
			break;

		case IDM_ETAT_ETAT:
			simu_envoi_aff (SIMU.aff_bal, M_SRV_ETAT, SRV_TYP_DEMANDE);
			break;

		case IDM_VISU_DEBUT:
			simu_envoi_aff (SIMU.aff_bal, M_AFF_VISU, SRV_TYP_DEBUT);
			break;

		case IDM_VISU_FIN:
			simu_envoi_aff (SIMU.aff_bal, M_AFF_VISU, SRV_TYP_FIN);
			break;

		case IDM_VISU_VISU:
			simu_envoi_aff (SIMU.aff_bal, M_AFF_VISU, SRV_TYP_DEMANDE);
			break;

		case ID_DEVICEINFO_START:
			simu_envoi_aff (SIMU.aff_bal, M_SRV_DEVICE_INFO, SRV_TYP_DEBUT);
			break;

		case ID_DEVICEINFO_STOP:
			simu_envoi_aff (SIMU.aff_bal, M_SRV_DEVICE_INFO, SRV_TYP_FIN);
			break;

		case ID_DEVICEINFO_GETDEVICEINFO:
			simu_envoi_aff (SIMU.aff_bal, M_SRV_DEVICE_INFO, SRV_TYP_GET);
			break;

		case IDM_ARRET_ARRET:
			simu_envoi_aff (SIMU.aff_bal, M_SRV_ARRET, SRV_TYP_DEMANDE);
			break;
	
		// Selecting MailBiox
		case IDM_BAL_A:
			SIMU.bal_id = SIMU.user[0].bal_id;
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_A,MF_CHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_B,MF_UNCHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_C,MF_UNCHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_D,MF_UNCHECKED);
			break;

		case IDM_BAL_B:
			SIMU.bal_id = SIMU.user[1].bal_id;
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_A,MF_UNCHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_B,MF_CHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_C,MF_UNCHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_D,MF_UNCHECKED);
			break;

		case IDM_BAL_C:
			SIMU.bal_id = SIMU.user[2].bal_id;
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_A,MF_UNCHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_B,MF_UNCHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_C,MF_CHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_D,MF_UNCHECKED);
			break;

		case IDM_BAL_D:
			SIMU.bal_id = SIMU.user[3].bal_id;
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_A,MF_UNCHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_B,MF_UNCHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_C,MF_UNCHECKED);
			CheckMenuItem(GetMenu(hDlg),IDM_BAL_D,MF_CHECKED);
			break;

		// Info DialogBox
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)ABOUT, hDlg, (DLGPROC)About);
			break;

		// Exit from aplication
		case IDM_EXIT:
			ExitApplication(hDlg);
			break;

		case IDCANCEL:
			ExitApplication(hDlg);
			break;

        }

    }
    return (FALSE);
}

/***********************************************************************\
*
*  FUNCTION: AddLBItem();
*
*  PURPOSE:  Add item into ListBox.
*
\************************************************************************/

void AddLBItem(HWND hDlg, char *item)
{
	SendDlgItemMessage( hDlg, IDL_MESS, LB_ADDSTRING, 0,
		(LPARAM) (LPSTR) item ) ;
	index=(int)SendDlgItemMessage( hDlg, IDL_MESS, LB_GETCOUNT ,
		0, 0 ) ;
	SendDlgItemMessage( hDlg, IDL_MESS, LB_SETCURSEL ,
		index-1, 0 ) ;
}

/************************************************************************\
*
*  FUNCTION: DisableMenu();
*
*  PURPOSE:  Disable menu to prevent user of starting menu option 
*            while waiting on user respond to messagebox.
*
\************************************************************************/

void DisableMenu(HWND hDlg)
{
	EnableMenuItem(GetMenu(hDlg),IDM_EXIT,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_ABOUT,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_ETAT_DEBUT,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_ETAT_FIN,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_ETAT_ETAT,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_DEBUT,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_DISPLAY,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_RESET,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_LIGHT_ON,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_FIN,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_LIGHT_OFF,MF_GRAYED);	
	EnableMenuItem(GetMenu(hDlg),IDM_DOP_DEBUT,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_DOP_FIN,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),IDM_ARRET_ARRET,MF_GRAYED);
	EnableMenuItem(GetMenu(hDlg),ID_SERVICES_DEVICEINFO,MF_GRAYED);
}

/************************************************************************\
*
*  FUNCTION: EnableMenu();
*
*  PURPOSE:  Enable menu after user respond on messagebox. 
*
\************************************************************************/

void EnableMenu(HWND hDlg)
{
	EnableMenuItem(GetMenu(hDlg),IDM_EXIT,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_ABOUT,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_ETAT_DEBUT,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_ETAT_FIN,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_ETAT_ETAT,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_DEBUT,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_DISPLAY,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_RESET,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_LIGHT_ON,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_FIN,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_AFFICHAGE_LIGHT_OFF,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_DOP_DEBUT,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_DOP_FIN,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),IDM_ARRET_ARRET,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),ID_SERVICES_DEVICEINFO,MF_ENABLED);
}

/************************************************************************\
*
*  FUNCTION: Display();
*
*  PURPOSE:  Formating message to display. 
*
\************************************************************************/

LRESULT CALLBACK Display(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int cnt=0;
	unsigned char buffer[MAX_PATH];
	struct_aff_message *p_message = &SIMU.message;

	switch (message)
	{
	case WM_INITDIALOG:
		//set limits to form controls
		memset(p_message, 0, sizeof(struct_aff_message));
		SendDlgItemMessage( hDlg, IDC_CLASS,		EM_SETLIMITTEXT, 1,	0 ) ;
		SendDlgItemMessage( hDlg, IDC_FARE,			EM_SETLIMITTEXT, 5, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_LABEL,		EM_SETLIMITTEXT, 40, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_PRIORITY,		EM_SETLIMITTEXT, 2, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_PAY_TYPE,		EM_SETLIMITTEXT, 5, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_PAID,			EM_SETLIMITTEXT, 5, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_SALEFARE,		EM_SETLIMITTEXT, 5, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_LIB1,			EM_SETLIMITTEXT, 18, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_LIB2,			EM_SETLIMITTEXT, 18, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_LIB3,			EM_SETLIMITTEXT, 18, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_SOFT_REV,		EM_SETLIMITTEXT, 18, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_PREC,			EM_SETLIMITTEXT, 5, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_AMOUNT_DUE,	EM_SETLIMITTEXT, 5, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_OVERPAY,		EM_SETLIMITTEXT, 5, 0 ) ;
		//set default values
		SetDlgItemText(hDlg, IDC_PRIORITY,	"5");
		SetDlgItemText(hDlg, IDC_PREC,		"100");
		SetDlgItemText(hDlg, IDC_PAY_TYPE,	"Cash");
		SetDlgItemText(hDlg, IDC_CLASS,		"1");
		SetDlgItemText(hDlg, IDC_FARE,		"8000");
		SetDlgItemText(hDlg, IDC_PAID,		"5550");
		SetDlgItemText(hDlg, IDC_SALEFARE,	"1234");
		SetDlgItemText(hDlg, IDC_LIB1,		"PLEASE PAY");
		SetDlgItemText(hDlg, IDC_LIB2,		"LANE OPENED");
		SetDlgItemText(hDlg, IDC_LIB3,		"GOOD TRIP!");
		SetDlgItemText(hDlg, IDC_SOFT_REV,	"Ver:1.0 04.08.2000");
		SetDlgItemText(hDlg, IDC_AMOUNT_DUE,"1325");
		SetDlgItemText(hDlg, IDC_OVERPAY,	"2589");
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			cnt=GetDlgItemText(hDlg,IDC_LABEL,buffer,41);
			if (cnt == 0)
			{
				MessageBox(hDlg,"Enter label!","Warning",MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
			else
			{
				strcpy_s(p_message->u.srv_affichage.demande.label, sizeof(p_message->u.srv_affichage.demande.label), buffer);
			}
			cnt=GetDlgItemText(hDlg,IDC_PRIORITY,buffer,3);
			if (cnt == 0)
			{
				MessageBox(hDlg,"Enter priority!","Warning",MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
			else
			{
				p_message->u.srv_affichage.demande.priorite = (unsigned char)atol(buffer);
			}
			cnt=GetDlgItemText(hDlg,IDC_PREC,buffer,6);
			if (cnt == 0)
			{
				MessageBox(hDlg,"Enter precision!","Warning",MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
			else
			{
				AFF_ECRIRE_DONNEE_LONG( p_message->u.srv_affichage.demande.precision_tarif,atol(buffer) );
			}

			GetDlgItemText(hDlg,IDC_CLASS,buffer,2);
			AFF_ECRIRE_DONNEE_STRING(p_message->u.srv_affichage.demande.vehicule.veh_class, buffer, NOYAU_GetPoolId("SIMUAFF") );
// 			AFF_ECRIRE_DONNEE_LONG(p_message->u.srv_affichage.demande.vehicule.class, atol(buffer) );

			GetDlgItemText(hDlg,IDC_FARE,buffer,6);
			AFF_ECRIRE_DONNEE_LONG( p_message->u.srv_affichage.demande.vehicule.fare, atol(buffer) );

			GetDlgItemText(hDlg,IDC_PAY_TYPE,buffer,6);
			AFF_ECRIRE_DONNEE_STRING( p_message->u.srv_affichage.demande.vehicule.payment_type,buffer,NOYAU_GetPoolId("SIMUAFF") );

			GetDlgItemText(hDlg,IDC_PAID,buffer,6);
			AFF_ECRIRE_DONNEE_LONG( p_message->u.srv_affichage.demande.vehicule.paid, atol(buffer) );

			GetDlgItemText(hDlg,IDC_SALEFARE,buffer,6);
			AFF_ECRIRE_DONNEE_LONG( p_message->u.srv_affichage.demande.vehicule.sale_fare, atol(buffer) );

			GetDlgItemText(hDlg,IDC_LIB1,buffer,20);
			AFF_ECRIRE_DONNEE_STRING( p_message->u.srv_affichage.demande.vehicule.libelle1,buffer,NOYAU_GetPoolId("SIMUAFF") );

			GetDlgItemText(hDlg,IDC_LIB2,buffer,20);
			AFF_ECRIRE_DONNEE_STRING( p_message->u.srv_affichage.demande.vehicule.libelle2,buffer,NOYAU_GetPoolId("SIMUAFF") );

			GetDlgItemText(hDlg,IDC_LIB3,buffer,20);
			AFF_ECRIRE_DONNEE_STRING( p_message->u.srv_affichage.demande.vehicule.libelle3,buffer,NOYAU_GetPoolId("SIMUAFF") );

			GetDlgItemText(hDlg,IDC_SOFT_REV,buffer,19);
			AFF_ECRIRE_DONNEE_STRING( p_message->u.srv_affichage.demande.soft_revision,buffer,NOYAU_GetPoolId("SIMUAFF") );

			GetDlgItemText(hDlg,IDC_AMOUNT_DUE,buffer,6);
			AFF_ECRIRE_DONNEE_LONG( p_message->u.srv_affichage.demande.vehicule.amount_due, atol(buffer) );
			
			GetDlgItemText(hDlg,IDC_OVERPAY,buffer,6);
			AFF_ECRIRE_DONNEE_LONG( p_message->u.srv_affichage.demande.vehicule.overpay, atol(buffer) );

			simu_envoi_aff (SIMU.aff_bal, M_AFF_AFFICHAGE, SRV_TYP_DEMANDE);
			
			EndDialog(hDlg, LOWORD(wParam));
			
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;

			return TRUE;					
		}
	}
    return FALSE;
}

/************************************************************************\
*
*  FUNCTION: Reset();
*
*  PURPOSE:  Display previous message 
*
\************************************************************************/

LRESULT CALLBACK Reset(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int cnt=0;
	char buffer[3];
	struct_aff_message *p_message = &SIMU.message;

	switch (message)
	{
	case WM_INITDIALOG:
		memset(p_message, 0, sizeof(struct_aff_message));
		SendDlgItemMessage( hDlg, IDC_PRIORITY2, EM_SETLIMITTEXT, 2, 0 ) ;
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			cnt=GetDlgItemText(hDlg,IDC_PRIORITY2,buffer,3);
			if (cnt == 0)
			{
				MessageBox(hDlg,"Enter priority!","Warning",MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
			else
			{
				p_message->u.srv_affichage.reset.priorite = (unsigned char)atol(buffer);
			}

			simu_envoi_aff (SIMU.aff_bal, M_AFF_AFFICHAGE, AFF_TYP_RESET);

			EndDialog(hDlg, LOWORD(wParam));
			
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
	}
    return FALSE;
}

/************************************************************************\
*
*  FUNCTION: Start_Espion();
*
*  PURPOSE:  Process starting of ESPION service
*
\************************************************************************/

LRESULT CALLBACK Start_Espion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		CheckRadioButton(hDlg,IDC_SERV1,IDC_SERV2,IDC_SERV1);
		CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO3,IDC_RADIO1);
		SIMU.espion_nature=IDC_SERV1;
		SIMU.espion_sens=IDC_RADIO1;
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam)==BN_CLICKED)
		{
			if ((LOWORD(wParam)>=IDC_SERV1) && (LOWORD(wParam)<=IDC_SERV2))
			{
				SIMU.espion_nature=LOWORD(wParam);
				CheckRadioButton(hDlg,IDC_SERV1,IDC_SERV2,LOWORD(wParam));
			}
			if ((LOWORD(wParam)>=IDC_RADIO1) && (LOWORD(wParam)<=IDC_RADIO3))
			{
				SIMU.espion_sens=LOWORD(wParam);
				CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO3,LOWORD(wParam));
			}
		}
		switch(LOWORD(wParam))
		{
		case IDC_ESPION_CANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
			break;

		case IDC_ESPION_OK:
			switch(SIMU.espion_nature)
			{
			case IDC_SERV1:
				SIMU.message.u.srv_espion.nature = 1;
				break;

			case IDC_SERV2:
				SIMU.message.u.srv_espion.nature = 2;
				break;
			}
			switch(SIMU.espion_sens)
			{
			case IDC_RADIO1:
				SIMU.message.u.srv_espion.sens = 1;
				break;

			case IDC_RADIO2:
				SIMU.message.u.srv_espion.sens = 2;
				break;

			case IDC_RADIO3:
				SIMU.message.u.srv_espion.sens = 3;
				break;
			}
		simu_envoi_aff (SIMU.aff_bal, M_SRV_ESPION, SRV_TYP_DEBUT);

		EndDialog(hDlg, LOWORD(wParam));
		
		return TRUE;
		
		break;
		}
		return TRUE;
	}
    return FALSE;
}
/**************MSU**************************/

/************************************************************************\
*
*  FUNCTION: BrowseDlgProc();
*
*  PURPOSE:  
*
\************************************************************************/

LRESULT CALLBACK BrowseDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    OPENFILENAME ofn;
	unsigned char lpszPathName[256];


	UNREFERENCED_PARAMETER( lParam );

	switch (message)
	{
		case WM_INITDIALOG:
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BROWSE:
					ZeroMemory(&ofn, sizeof(ofn));
					lpszPathName[0] = '\0';
	
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = GetActiveWindow();
					ofn.lpstrFile = (LPSTR)lpszPathName;
					ofn.nMaxFile = sizeof(lpszPathName);
					ofn.lpstrFilter = "Text files(*.txt)\0*.txt\0All files(*.*)\0*.*\0\0";

					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = (unsigned long)NULL;
					ofn.lpstrInitialDir = NULL;

					GetOpenFileName(&ofn);

					strcpy_s(szTmp, sizeof(szTmp), ofn.lpstrFile);
					SetDlgItemText(hDlg, IDC_FILENAME, szTmp);
					strcpy_s(SIMU.szTmp, sizeof(SIMU.szTmp), szTmp);
					break;
				
				case IDOK:
					if (strlen(SIMU.szTmp) == 0)
					{
						MessageBox(hDlg,"Enter file name", "Error", MB_OK);
						break;
					}
					strcpy_s(SIMU.message.u.srv_affichage.newFile.FilePath, sizeof(SIMU.message.u.srv_affichage.newFile.FilePath), SIMU.szTmp);
					simu_envoi_aff (SIMU.aff_bal, M_AFF_AFFICHAGE, AFF_TYP_NEW_FILE);

					break;

				case IDCANCEL:
					EndDialog(hDlg, TRUE);
					break;
			}
			break;

		case WM_CLOSE :
			EndDialog(hDlg, TRUE);
			break;
	}

	return (FALSE);
}

/*******************************************/

/************************************************************************\
*
*  FUNCTION: TestDisplay();
*
*  PURPOSE:  Display test message 
*
\************************************************************************/

LRESULT CALLBACK TestDisplay(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int cnt=0;
	unsigned char buffer[50+1];
	struct_aff_message *p_message = &SIMU.message;

	switch (message)
	{
	case WM_INITDIALOG:
		memset(p_message, 0, sizeof(struct_aff_message));

		SendDlgItemMessage( hDlg, IDC_PRIORITY3, EM_SETLIMITTEXT, 2, 0 ) ;
		SendDlgItemMessage( hDlg, IDC_TEST_LABEL, EM_SETLIMITTEXT, 50, 0 ) ;
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			cnt=GetDlgItemText(hDlg,IDC_TEST_LABEL,buffer,51);
			if (cnt == 0)
			{
				MessageBox(hDlg,"Enter Test label!","Warning",MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
			else
			{
				strcpy_s(p_message->u.srv_test.ligne, sizeof(p_message->u.srv_test.ligne), buffer);
			}
			cnt=GetDlgItemText(hDlg,IDC_PRIORITY3,buffer,3);
			if (cnt == 0)
			{
				MessageBox(hDlg,"Enter priority!","Warning",MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
			else
			{
				p_message->u.srv_test.priorite = (unsigned char)atol(buffer);
			}
			simu_envoi_aff (SIMU.aff_bal, M_AFF_AFFICHAGE, SRV_TYP_TEST);

			EndDialog(hDlg, LOWORD(wParam));
			
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
	}
    return FALSE;
}

/************************************************************************\
*
*  FUNCTION: About();
*
*  PURPOSE:  Process about DialogBox 
*
\************************************************************************/

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

/************************************************************************\
*
*  FUNCTION: ExitAplication();
*
*  PURPOSE:  Exit aplication.
*
\************************************************************************/

void ExitApplication(HWND hDlg)
{
	int tstex;

	DisableMenu(hDlg);
	tstex= MessageBox( NULL, "Stop tasks and exit aplication?",
		"Confirmation", MB_OKCANCEL | MB_ICONQUESTION | 
		MB_SYSTEMMODAL );
	if (tstex==IDOK)
	{

		ArretModule(hDlg);
		EndDialog (hDlg, TRUE);
	}
	else
		EnableMenu(hDlg);
}


/************************************************************************\
*
*  FUNCTION: PerformanceTest();
*
*  PURPOSE:
*
\************************************************************************/

LRESULT CALLBACK PerformanceTest(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:
		// Disable STOP button
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), FALSE);
		// set label lentgth to AFF_MAX_LABEL = 40
		SendDlgItemMessage(hDlg, IDC_EDIT1, EM_SETLIMITTEXT, 40, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT2, EM_SETLIMITTEXT, 40, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT3, EM_SETLIMITTEXT, 40, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT4, EM_SETLIMITTEXT, 40, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT5, EM_SETLIMITTEXT, 40, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT6, EM_SETLIMITTEXT, 5, 0);
		// set delay to 1000 ms
		SetDlgItemInt(hDlg, IDC_EDIT6, 1000, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_START:
			if (StartPerformanceTest(hDlg) == TRUE)
			{
				//disable START button and enable STOP button
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), FALSE);
			}
			break;

		case IDC_BUTTON_STOP:
			DoStop(hDlg);
			break;

		case IDCANCEL:
			if (SIMU.bPerfomTestStarted)
				DoStop(hDlg);
			EndDialog(hDlg, TRUE);
			break;
		}
		break;

	case WM_CLOSE:
		if (SIMU.bPerfomTestStarted)
			DoStop(hDlg);
		EndDialog(hDlg, TRUE);
		break;

	case WM_TIMER:
		switch (wParam)
		{
		case IDT_TIMER_DELAY:
			DoPerformanceTestStep();
			break;
		}
		break;

	}

	return (FALSE);
}

/*******************************************/

/************************************************************************\
*
*  FUNCTION: StartPerformanceTest();
*
*  PURPOSE:  
*
\************************************************************************/

BOOL StartPerformanceTest(HWND hDlg)
{
	unsigned char		szTemp[MAX_PATH] = {0};
	UINT				uiRes = 0;
	UINT				uElapse = 0;

	// get first label, if missing, abort the test with warning
	uiRes = GetDlgItemText(hDlg, IDC_EDIT1, SIMU.aszTestLabels[0], 40);
	if (uiRes == 0)
	{
		MessageBox(hDlg, "Enter Label 1!", "Warning", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	// get timer delay, if less than 100 ms abort with warning
	uElapse = GetDlgItemInt(hDlg, IDC_EDIT6, NULL, FALSE);
	if (uElapse < 100)
	{
		MessageBox(hDlg, "Cycle Delay minimum value is 100 ms!", "Warning", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	// start timer
	if (!SetTimer(hDlg, IDT_TIMER_DELAY, uElapse, NULL))
	{
		MessageBox(hDlg, "SetTimer() function failed!", "Warning", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;

	}

	// get other labels if any
	GetDlgItemText(hDlg, IDC_EDIT2, SIMU.aszTestLabels[1], 40);
	GetDlgItemText(hDlg, IDC_EDIT3, SIMU.aszTestLabels[2], 40);
	GetDlgItemText(hDlg, IDC_EDIT4, SIMU.aszTestLabels[3], 40);
	GetDlgItemText(hDlg, IDC_EDIT5, SIMU.aszTestLabels[4], 40);

	// subscribe to AFFICHAGE service
	// (if already suscribed => it doesn't matter)
	simu_envoi_aff(SIMU.aff_bal, M_AFF_AFFICHAGE, SRV_TYP_DEBUT);
	SIMU.bPerfomTestStarted = TRUE;

	return TRUE;
}

BOOL StopPerformanceTest(HWND hDlg)
{
	unsigned char		szTemp[MAX_PATH] = { 0 };
	UINT				uiIdx = 0;
	UINT				uElapse = 0;
	struct_aff_message *p_message = &SIMU.message;

	// reset priority
	p_message->u.srv_affichage.reset.priorite = AFF_MAX_PRIORITY;
	simu_envoi_aff(SIMU.aff_bal, M_AFF_AFFICHAGE, AFF_TYP_RESET);

	// unsubscribe from AFFICHAGE service
	simu_envoi_aff(SIMU.aff_bal, M_AFF_AFFICHAGE, SRV_TYP_FIN);
	SIMU.bPerfomTestStarted = FALSE;

	// Stop timer
	if (!KillTimer(hDlg, IDT_TIMER_DELAY))
	{
		MessageBox(hDlg, "KillTimer() function failed!", "Warning", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	return TRUE;
}

void DoPerformanceTestStep(void)
{
	struct_aff_message	*p_message = &SIMU.message;
	unsigned char		szTemp[MAX_PATH] = { 0 };
	static UINT			uiIdx = 0;
	int					i = 0;

	// fill the message common part

	// priority: max one, it should be 7
	p_message->u.srv_affichage.demande.priorite = AFF_MAX_PRIORITY;

	// tariff precision, set to 2
	AFF_ECRIRE_DONNEE_LONG(p_message->u.srv_affichage.demande.precision_tarif, 100);

	// class, set to 1
	AFF_ECRIRE_DONNEE_STRING(p_message->u.srv_affichage.demande.vehicule.veh_class, "1", NOYAU_GetPoolId("SIMUAFF"));

	// tariff, set to "12.34"
	AFF_ECRIRE_DONNEE_LONG(p_message->u.srv_affichage.demande.vehicule.fare, 1234);

	// payment type, set to "CASH"
	AFF_ECRIRE_DONNEE_STRING(p_message->u.srv_affichage.demande.vehicule.payment_type, "CASH", NOYAU_GetPoolId("SIMUAFF"));

	// paid value, set to "10.00"
	AFF_ECRIRE_DONNEE_LONG(p_message->u.srv_affichage.demande.vehicule.paid, 999);

	// sale fare, set to "98.76"
	AFF_ECRIRE_DONNEE_LONG(p_message->u.srv_affichage.demande.vehicule.sale_fare, 9876);

	// label 1, set to "LABEL 1"
	AFF_ECRIRE_DONNEE_STRING(p_message->u.srv_affichage.demande.vehicule.libelle1, "LABEL 1", NOYAU_GetPoolId("SIMUAFF"));
	// label 2, set to "LABEL 2"
	AFF_ECRIRE_DONNEE_STRING(p_message->u.srv_affichage.demande.vehicule.libelle2, "LABEL 2", NOYAU_GetPoolId("SIMUAFF"));
	// label 3, set to "LABEL 3"
	AFF_ECRIRE_DONNEE_STRING(p_message->u.srv_affichage.demande.vehicule.libelle3, "LABEL 3", NOYAU_GetPoolId("SIMUAFF"));

	// soft revision, set to "v.1.2.3"
	AFF_ECRIRE_DONNEE_STRING(p_message->u.srv_affichage.demande.soft_revision, "v.1.2.3", NOYAU_GetPoolId("SIMUAFF"));

	// amount due, set to "1.00"
	AFF_ECRIRE_DONNEE_LONG(p_message->u.srv_affichage.demande.vehicule.amount_due, 100);

	// overpay, set to "0.50"
	AFF_ECRIRE_DONNEE_LONG(p_message->u.srv_affichage.demande.vehicule.overpay, 50);

	while (SIMU.aszTestLabels[uiIdx][0] == '\0')
	{
		if (++uiIdx >= NB_TEST_LABELS)
			uiIdx = 0;
		
		// to avoid infinite loop 
		if (++i > NB_TEST_LABELS)
			break;
	}

	strcpy_s(p_message->u.srv_affichage.demande.label, sizeof(p_message->u.srv_affichage.demande.label), SIMU.aszTestLabels[uiIdx]);
	simu_envoi_aff(SIMU.aff_bal, M_AFF_AFFICHAGE, SRV_TYP_DEMANDE);
	
	if (++uiIdx >= NB_TEST_LABELS)
		uiIdx = 0;
}

void DoStop(HWND hDlg)
{
	if (StopPerformanceTest(hDlg))
	{
		//enable START button and disable STOP button
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), TRUE);
	}
}
