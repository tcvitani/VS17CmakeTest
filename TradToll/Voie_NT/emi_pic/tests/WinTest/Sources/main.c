/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   .																 */
/* FILE:     main.c											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
#include <global.h>
/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/
LONG DialogProc(
				HWND hwndDlg,  // handle to dialog box
				UINT uMsg,     // message
				WPARAM wParam, // first message parameter
				LPARAM lParam) // second message parameter
				;

VOID EventRequestReemission( VOID );
VOID InitWindow( VOID );
VOID InitServices( VOID );
VOID RequestReemissionById( VOID  );
/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/
/*****************************************************************************/
/*SYNTAX: int APIENTRY WinMain( HINSTANCE hInstance,                         */
/*								HINSTANCE hPrevInstance,                     */
/*								LPSTR     lpCmdLine,                         */
/*								int       nCmdShow)                          */
/*===========================================================================*/
/*TYPE:   Main function.                                                     */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Application entry point function. The function                 */
/*            initializes the application.                                   */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HINSTANCE hInstance     - Handle to the current instance of    */
/*                                      the application.                     */
/*      (IN ) HINSTANCE hPrevInstance - Handle to the previous instance of   */
/*                                      the application.                     */
/*      (IN ) LPSTR     lpCmdLine     - Pointer to a null-terminated string  */
/*                                      specifying the command line for the  */
/*                                      application, excluding the program   */
/*                                      name.                                */
/*      (IN ) int       nCmdShow      - Specifies how the window is to be    */
/*                                      shown.                               */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  0                     The function succeeds.                             */
/*  non zero              The function does not succeed.                     */
/*****************************************************************************/
int APIENTRY WinMain(	HINSTANCE hInstance,
						HINSTANCE hPrevInstance,
						LPSTR     lpCmdLine,
						int       nCmdShow )
{
	HMODULE
		hMod;
	

	hMod = LoadLibrary( TEXT( DLL_FILE ) ) ;

	if( hMod == NULL )
	{
		MessageBox( NULL, "Error loading dll", "Error", MB_OK);
		return 0;
	}

	SIMU.LpFncMODLance = (LPFNC_LANCE)GetProcAddress( hMod, FNC_MOD_LANCE ); 
	SIMU.LpFncMODArret = (LPFNC_ARRET)GetProcAddress( hMod, FNC_MOD_ARRET );
	
	if( SIMU.LpFncMODLance == NULL ||
		SIMU.LpFncMODArret == NULL)
	{
		MessageBox( NULL, "Error loading dll", "Error", MB_OK);
		return 0;
	}
	
	SIMU.hInst  = hInstance;
	SIMU.bRun	= TRUE;

	strcpy( SIMU.MailBoxName, MODULE_MAILBOX );

	if( OpenMailBox() != TRUE )
		ExitBad();	

	DialogBox(	SIMU.hInst,
				MAKEINTRESOURCE(IDD_MAIN),
				NULL,
				(DLGPROC)DialogProc);
	
	return FALSE;
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
LONG DialogProc(
							HWND hwndDlg,  // handle to dialog box
							UINT uMsg,     // message
							WPARAM wParam, // first message parameter
							LPARAM lParam) // second message parameter
{
	int
		i;

	switch( uMsg )
	{
		case WM_INITDIALOG:

			SIMU.hDlg = hwndDlg;

			i = 0;
			SIMU.from_to_wnd[0][i++] = GetDlgItem( hwndDlg, IDC_FROM_YEAR );					
			SIMU.from_to_wnd[0][i++] = GetDlgItem( hwndDlg, IDC_FROM_MONTH);
			SIMU.from_to_wnd[0][i++] = GetDlgItem( hwndDlg, IDC_FROM_DAY);
			SIMU.from_to_wnd[0][i++] = GetDlgItem( hwndDlg, IDC_FROM_HOUR);
			SIMU.from_to_wnd[0][i++] = GetDlgItem( hwndDlg, IDC_FROM_MINUTE);
			SIMU.from_to_wnd[0][i++] = GetDlgItem( hwndDlg, IDC_FROM_SECOND);
			SIMU.from_to_wnd[0][i++] = GetDlgItem( hwndDlg, IDC_FROM_MILLISECOND);
			
			i = 0;
			SIMU.from_to_wnd[1][i++] = GetDlgItem( hwndDlg, IDC_TO_YEAR);
			SIMU.from_to_wnd[1][i++] = GetDlgItem( hwndDlg, IDC_TO_MONTH);
			SIMU.from_to_wnd[1][i++] = GetDlgItem( hwndDlg, IDC_TO_DAY);
			SIMU.from_to_wnd[1][i++] = GetDlgItem( hwndDlg, IDC_TO_HOUR);
			SIMU.from_to_wnd[1][i++] = GetDlgItem( hwndDlg, IDC_TO_MINUTE);
			SIMU.from_to_wnd[1][i++] = GetDlgItem( hwndDlg, IDC_TO_SECOND);
			SIMU.from_to_wnd[1][i++] = GetDlgItem( hwndDlg, IDC_TO_MILLISECOND);

			InitWindow();
			InitServices();
			break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_REQOEST_REEMISSION:
					EventRequestReemission();
					break;
				case IDC_REQOEST_REEMISSION_ID:
					RequestReemissionById();
					break;
				default:
					break;
			}
			break;

		case WM_CLOSE:
			PostQuitMessage( FALSE );
			break;
			
		default:
			break;
	}
			
	return FALSE;
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID EventRequestReemission( VOID )
{
	int
		i,
		j;
	
	int
		a[2][NB_TIME_ELEMENTS];

	CHAR
		szBuffer[MAX_PATH];

	LPSYSTEMTIME
		lpTime;
	

	for( i = 0; i < 2; i++ )
	{
		for( j = FIRST_TIME_ELEMENT; j < NB_TIME_ELEMENTS; j++ )
		{
			GetWindowText( SIMU.from_to_wnd[i][j], szBuffer, MAX_PATH );
			a[i][j] = atol( szBuffer );
		}
	}

	for( i = 0; i < 2; i++ )
	{
		switch( i )
		{
		case 0:
			lpTime = &(SIMU.sFromTime);
			break;
			
		case 1:
			lpTime = &(SIMU.sToTime);
			break;
			
		default:
			return;
		}
		
		for( j = FIRST_TIME_ELEMENT; j < NB_TIME_ELEMENTS; j++ )
		{
			switch( j )
			{
			case YEAR:
				lpTime->wYear = a[i][j];
				break;
			case MONTH:
				lpTime->wMonth = a[i][j];
				break;
			case DAY:
				lpTime->wDay = a[i][j];
				break;
			case HOUR:
				lpTime->wHour = a[i][j];
				break;
			case MINUTE:
				lpTime->wMinute = a[i][j];
				break;
			case SECOND:
				lpTime->wSecond = a[i][j];
				break;
			case MILLISECOND:
				lpTime->wMilliseconds = a[i][j];
				break;
			default:
				break;
			}
		}
	}

	SimuEnvoi( M_EMI_PIC_RESTITUTION, M_EMI_PIC_TYP_RESTIT_BY_DATETIME );
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID RequestReemissionById( VOID )
{
	CHAR
		szText[MAX_PATH];
	HWND
		hwText = NULL;

	hwText = GetDlgItem( SIMU.hDlg, IDC_EDIT1 );

	GetWindowText( hwText, szText, MAX_PATH );
	SIMU.lRecordID = atol( szText );
	SimuEnvoi( M_EMI_PIC_RESTITUTION, M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID );
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID InitWindow( VOID )
{
	int
		a[2][NB_TIME_ELEMENTS] = 
								{
									{ 2008, 1, 1, 0, 0, 0 },
									{ 2008, 1, 1, 0, 0, 0 },
								},
		i,
		j;

	CHAR
		szBuffer[MAX_PATH];

	for( i = 0; i < 2; i++ )
	{
		for( j = FIRST_TIME_ELEMENT; j < NB_TIME_ELEMENTS; j++ )
		{
			sprintf( szBuffer, "%d", a[i][j] );
			SetWindowText( SIMU.from_to_wnd[i][j], szBuffer );
		}
	}
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
VOID InitServices( VOID )
{
	SimuEnvoi( M_EMI_PIC_RESTITUTION, SRV_TYP_DEBUT );	
	SimuEnvoi( M_SRV_ETAT, SRV_TYP_DEBUT );	
}
/*-------------------------------- END OF FILE ------------------------------*/
