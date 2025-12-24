/************ (v) 2009 CSEE-Peage -   All rights reserved ********************/
/*																			 */
/* --------------------------------------------------------------------------*/
/* MODULE:																	 */
/* FILE:     winmain.c														 */
/* LANGUAGE: C																 */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:																	 */
/*****************************************************************************/
/*---------------------------------- INCLUDES:  -----------------------------*/

#include <simu.h>
#include <shlobj.h>	// Browse folder interface
/*---------------------------------- RESERVED:  -----------------------------*/
#include <memclass.h>
/*---------------------------------- EXTERNALS: -----------------------------*/

/*---------------------------------- DEFINES:   -----------------------------*/
#define DLL_FILE					"CSR_EMI_PIC10_VS12_X64.dll"

#define EMI_PIC_REG_KEYv_LOCAL_DIR	"LocalDirectory"
#define BASE_KEY_PATH				"SOFTWARE\\CSRoute\\Maintenance\\Languages\\"
#define REG_KEYv_TITLE				"TitleLabel"
/*---------------------------------- TYPEDEFS:  -----------------------------*/

/*---------------------------------- FUNCTIONS: -----------------------------*/
PRIVATE int APIENTRY MainDlgProc( HWND	hDlg,
								  WORD	wMsg, 
								  LONG	wParam, 
								  LONG	lParam );

PRIVATE BOOL ReadEmiPicRegistry( VOID );
PRIVATE VOID InitTranslations( VOID );
PRIVATE BOOL SubscribedToService( VOID );
PRIVATE BOOL InitDialogs( VOID );
PRIVATE VOID HandleBrowse( HWND hDlg );
PRIVATE VOID HandleStart( VOID );
PRIVATE VOID HandleCancel( VOID );
PRIVATE BOOL AllTargetPathsSet( VOID );
PRIVATE VOID UpdateStatusText( VOID );
/*---------------------------------- VARIABLES: -----------------------------*/
BOOL Closing = FALSE;
/*---------------------------------- CODE:      -----------------------------*/
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
	MSG 
		msg ;
	HACCEL
		hacTable;
	HMODULE
		hMod;
	LONG
		i;

	UNREFERENCED_PARAMETER( nCmdShow );

	hMod = LoadLibrary( TEXT( DLL_FILE ) ) ;

	if( hMod == NULL )
		return 0;

	SIMU.LpFncMODLance = (PMOD_PROC_LANCE)GetProcAddress( hMod, MOD_PROC_LANCE ); 
	SIMU.LpFncMODArret = (PMOD_PROC_ARRET)GetProcAddress( hMod, MOD_PROC_ARRET );

	if( SIMU.LpFncMODLance == NULL ||
		SIMU.LpFncMODArret == NULL)
	{
		return 0;
	}

	InitCommonControls();
	StartUsingLanguages( BASE_KEY_PATH );

	InitBrushes();
				
	SIMU.hInst  = hInstance;
	SIMU.bRun	= TRUE;	

	SIMU.hWnd = hwndInitWindow( hInstance, nCmdShow );
	
	if( SIMU.hWnd == NULL )
	{
		return FALSE;	
	}

	if( ReadEmiPicRegistry() != TRUE )
	{
		return FALSE;
	}

	AddNewTranslationObject( NULL, NULL, TXT_ERR_ERR );
	AddNewTranslationObject( NULL, NULL, TXT_ERROR_START_MODULE );	

	if( !OpenMailBox() )
	{
		return FALSE;	
	}

	if( InitDialogs() != TRUE )
	{
		return FALSE;	
	}

	strcpy( SIMU.MAINT_bal_name, APP_BAL_NAME );

	// In case of app crash, it is possible that mailbox is still registerd
	// Clean it
	SupprimeBAL( SIMU.MAINT_bal_name );	

	// Publish own mailbox
	SIMU.MAINT_bal_id = PublieBAL( SIMU.MAINT_bal_name, 0 );

	if( SIMU.MAINT_bal_id == 0 )
	{// Failed to open own mailbox
		return FALSE;
	}

	if( LanceModule() != TRUE )
	{// Failed to launch communication thread
		// Close own mailbox
		SupprimeBAL( SIMU.MAINT_bal_name );

		return FALSE;
	}

	// Init translations
	InitTranslations();
	
	// Switch to default language
	SwitchDefaultLanguage();
	


	// Set application text
	// Set application text
	UpdateTranslations( SIMU.hWnd );


	// Subscribe to module services
	for( i = 0; i < SIMU.NbModuleInstances; i++ )
	{
		// Subscribe to maintenance mode service
		SimuEnvoi( SIMU.ModuleData[i].MailboxID, M_EMI_PIC_MAINT_MODE, SRV_TYP_DEBUT );
	}
	
	SetFocus( SIMU.hDlg );			
	

	// Load accelerator table
	hacTable = LoadAccelerators( hInstance,					//Handle to application instance
								 MAKEINTRESOURCE(IDR_ACC) );//Address of table-name string
		
	if(hacTable == NULL)
	{// Failed to load accelerator table
		return FALSE;
	}
		
	// Translate Accelerator and message
	while( SIMU.bRun )
	{
		if (GetMessage (&msg, NULL, 0, 0))
		{
			if(!TranslateAccelerator(
				SIMU.hDlg,				//Handle to destination window
				hacTable,				//Handle to accelerator table
				&msg					//Address of structure with message
				) )
			{
				if(!IsDialogMessage(SIMU.hDlg, &msg))
				{
					TranslateMessage (&msg) ;
					DispatchMessage (&msg) ;
				}
			}
		}    
		else
			SIMU.bRun=FALSE;
	}
	
	EndUsingLanguages();
	FreeBrushes();

	return msg.wParam ;
}
/*****************************************************************************/
/*SYNTAX: int APIENTRY InitDlgProc( HWND hDlg,				                 */
/*									WORD wMsg,							     */
/*									LONG wParam,							 */
/*									LONG lParam)			                 */
/*===========================================================================*/
/*TYPE:   Local function.													 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*            The function processes the messages in the main				 */
/*            dialog box queue.												 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*      (IN ) HWND hDlg    - Identifies the dialog box.						 */
/*      (IN ) WORD wMsg    - Specifies the message.							 */
/*      (IN ) LONG wParam  - Specifies additional message-specific			 */
/*                           information.									 */
/*      (IN ) LONG lParam  - Specifies additional message-specific			 */
/*                           information.									 */
/*===========================================================================*/
/*  Return                Description										 */
/*---------------------------------------------------------------------------*/
/*  0                     The function does not processes the				 */
/*                        message.											 */
/*  non zero              The function processes the message.				 */
/*****************************************************************************/
PRIVATE int APIENTRY MainDlgProc(HWND hDlg,
								 WORD wMsg, 
								 LONG wParam, 
								 LONG lParam)
{
	UNREFERENCED_PARAMETER( lParam );

	switch( wMsg )
	{
		case WM_INITDIALOG:
			break;
			
		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_BROWSE:
					HandleBrowse( hDlg );
					break;

				case IDSTART:
					HandleStart();
					break;

				case IDCANCEL:
					PostMessage( SIMU.hDlg, WM_CLOSE, 0, 0 );
					break;

				case IDA_F2:
					// Switch to next language
					SwitchNextLanguage();
					// Update application text
					UpdateTranslations( SIMU.hWnd );
					UpdateStatusText();
					break;

				default:
					break;
			}
			break;

		case WM_CLOSE :
			if( !Closing )
			{
				LONG
					i,
					j;

				// Disable cancel
				EnableWindow( GetDlgItem( SIMU.hDlg, IDCANCEL ), FALSE );
				
				for( j = 0; j < SIMU.NbModuleInstances; j++ )
				{// For each module instance
					// Unsubscribe from services
					for( i = 0; i < NB_EMI_PIC_SERVICE+1; i++ )
					{// Check for each service if we are subscribed
						if( SIMU.ModuleData[j].SubscribedServices[i] != FALSE )
						{// Unsubscribe from service
							SimuEnvoi( SIMU.ModuleData[j].MailboxID, (short int)i, SRV_TYP_FIN );
						}
					}

					// If currentry working, send cancel message
					if( SIMU.ModuleData[j].Completed != TRUE )
					{
						SimuEnvoi( SIMU.ModuleData[j].MailboxID, M_EMI_PIC_MAINT_MODE, M_EMI_PIC_MAINT_MODE_STOP );
					}							
				}

				Closing = TRUE;				
			}

			if( SubscribedToService() != TRUE )	// Not subscribed to any service
			{
				SIMU.fin = TRUE;
				
			}
			else
				return TRUE;

			if( wParam == 1 )
			{
				ExitApplication( SIMU.hDlg );
			}
			
			break;
			
		case WM_CTLCOLORSTATIC:
			{
				int
					iIconType = ICON_NONE;

				if( SIMU.hWndStatusIcon != (HWND)lParam)
				{// Handle only icon
					break;
				}

				switch( SIMU.enAppStatus )
				{
					case APP_STATUS_INITIALIZING:
					case APP_STATUS_WORKING:
						iIconType = ICON_WAITING;
						break;
						
					case APP_STATUS_READY:
					case APP_STATUS_COMPLETED:
						iIconType = ICON_OK;			
						break;
					default:
						break;		
				}
				
				switch( iIconType )
				{
					case ICON_NONE:
						break;
					default:
						if( iIconType < FIRST_ICON || iIconType > LAST_ICON )
							break;

						return (INT)SetIcon( (HWND)lParam, iIconType );
						break;
				}
			}
			break;
	}
	return (FALSE);
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
PRIVATE VOID InitTranslations( VOID )
{
	LONG
		i;

	AddNewTranslationObject( SIMU.hWnd, SIMU.hWnd, TXT_MAIN_TOOL_NAME );
	
	for( i = 0; i < SIMU.NbModuleInstances; i++ )
	{
		AddNewTranslationObject( GetDlgItem( SIMU.ModuleData[i].hDlg, IDC_TXT_SOURCE  ), SIMU.hWnd, TXT_SOURCE );
		AddNewTranslationObject( GetDlgItem( SIMU.ModuleData[i].hDlg, IDC_TXT_DESTINATION ), SIMU.hWnd, TXT_DESTINATION );
		AddNewTranslationObject( GetDlgItem( SIMU.ModuleData[i].hDlg, IDC_BROWSE ), SIMU.hWnd, TXT_BROWSE );
		AddNewTranslationObject( GetDlgItem( SIMU.ModuleData[i].hDlg, IDC_TITLE ), SIMU.hWnd, SIMU.ModuleData[i].szTitleLabel );
	}

	AddNewTranslationObject( GetDlgItem( SIMU.hDlg, IDC_TXT_STATUS ), SIMU.hWnd, TXT_STATUS );
	AddNewTranslationObject( GetDlgItem( SIMU.hDlg, IDSTART ), SIMU.hWnd, TXT_START );
	AddNewTranslationObject( GetDlgItem( SIMU.hDlg, IDCANCEL ), SIMU.hWnd, TXT_CANCEL );

	AddNewTranslationObject( NULL, NULL,  TXT_READY );
	AddNewTranslationObject( NULL, NULL,  TXT_WORKING );
	AddNewTranslationObject( NULL, NULL,  TXT_COMPLETED );
	AddNewTranslationObject( NULL, NULL,  TXT_INITIALIZING );
	AddNewTranslationObject( NULL, NULL,  TXT_SELECT_DESTINATION );
	AddNewTranslationObject( NULL, NULL,  TXT_FILES_TRANSFERED );
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
PRIVATE BOOL SubscribedToService( VOID )
{
	LONG
		i,
		j;
	
	for( j = 0; j < SIMU.NbModuleInstances; j++ )
	{// For each module instance		
		for( i = 0; i < NB_EMI_PIC_SERVICE+1; i++ )
		{// For each module service
			// Check if subscribed
			if( SIMU.ModuleData[j].SubscribedServices[i] != FALSE )
			{
				return TRUE;
			}
		}
	}
	// Not subscribed to single service of any module instance
	return FALSE;
}
/*******************************************************************/
/*SYNTAX: void ExitApplication(HWND hDlg)                          */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*             This function closes the test application after     */
/*             confirmation.                                       */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      (IN )  HWND hDlg   - Dialog box handle.                    */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PROTECTED void ExitApplication(HWND hDlg)
{ 
//	LONG
//		i;

	SIMU.fin=TRUE;
	
	ArretModule( hDlg );

	// Debug
	// Calling arret can cause module to hang, skip it
	// Module will come down with application anyway
//	for( i = 0; i < SIMU.NbModuleInstances; i++ )
//	{
//		if( !SIMU.ModuleData[i].bModuleWasRunning )
//		{
//			SIMU.LpFncMODArret( SIMU.ModuleData[i].MailboxID );
//		}		
//	}
	// /Debug
	
	SIMU.bRun = FALSE;
	PostQuitMessage( 0 );
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
PRIVATE BOOL ReadEmiPicRegistry( VOID )
{
	CHAR
		szKey[MAX_PATH],
		szInstanceKey[MAX_PATH],
		szSubkey[MAX_PATH],
		szSourceDir[MAX_PATH],
		szTitle[MAX_PATH];
	LONG
		i,
		lTitleLen;
	DWORD
		ResultEnumerate,
		Index,
		SubkeyLen,
		SourceDirLen;
	
	sprintf( szKey, "%s%s%s%s%s",
		CSR_REG_KEYn_CSRBASE,
		CSR_REG_KEYn_LANE_BASE,
		CSR_REG_KEYn_CONFIG,
		MOD_REG_KEYn_MODULES,
		EMI_REG_KEYn_ModEMI_PIC );

	Index = 0;
	
	// Read all configured instances of EMI PIC module
	for( i = 0; i < EMI_PIC_INSTANCE_MAX; i++ )
	{
		SubkeyLen = MAX_PATH;
		ResultEnumerate = REG_Enum_Cles( CSR_REG_KEYi_ROOT,
										szKey,
										Index,
										szInstanceKey,
										&SubkeyLen );

		if( ResultEnumerate != ERROR_SUCCESS )
		{
			if( ResultEnumerate != ERROR_NO_MORE_ITEMS )
			{// There was some kind of real error while reading emi pic keys
				return FALSE;
			}
			else
			{// Not a real error, simply no more subkeys to read
				return TRUE;
			}
		}

		strcpy( SIMU.ModuleData[Index].szMailboxName, szInstanceKey );

		_makepath( szSubkey, NULL, szKey, szInstanceKey, NULL );

		SourceDirLen = MAX_PATH;
		REG_Lire_Chaine( CSR_REG_KEYi_ROOT,
			szSubkey,
			EMI_PIC_REG_KEYv_LOCAL_DIR,
			szSourceDir,
			&SourceDirLen );

		strcpy( SIMU.ModuleData[Index].szSourceDir, szSourceDir );

		lTitleLen = MAX_PATH;
		if( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, szSubkey, REG_KEYv_TITLE, szTitle, &lTitleLen ) != ERROR_SUCCESS )
		{
			strcpy( SIMU.ModuleData[Index].szTitleLabel, "Undefined" );
		}
		else
		{
			strcpy( SIMU.ModuleData[Index].szTitleLabel, szTitle );
		}
		

		SIMU.NbModuleInstances++;
		Index++;
	}

	return TRUE;
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
PRIVATE BOOL InitDialogs( VOID )
{
	LONG
		i,
		Top;
	RECT
		sRect;


	Top = 0;

	for( i = 0; i < SIMU.NbModuleInstances; i++ )
	{
		// Create simple dialog
		SIMU.ModuleData[i].hDlg = CreateDialog( (HANDLE)SIMU.hInst,	// Instance
			MAKEINTRESOURCE(IDD_MAIN),								// Dialog box template name
			SIMU.hWnd,												// Handle to parent window
			(DLGPROC)MainDlgProc);									// Dialog box procedure
		
		if(SIMU.ModuleData[i].hDlg == NULL)
		{
			return FALSE;
		}

		SetWindowPos( SIMU.ModuleData[i].hDlg,
			HWND_NOTOPMOST,
			0, Top,
			0, 0, 
			SWP_NOZORDER|SWP_NOSIZE );		

		GetClientRect( SIMU.ModuleData[i].hDlg, &sRect );

		Top += sRect.bottom;
		
		ShowWindow( SIMU.ModuleData[i].hDlg, SW_SHOWNORMAL );

		// NOTE : Module configuration MUST be read at his point
		SetWindowText( GetDlgItem( SIMU.ModuleData[i].hDlg, IDC_TARGET ),
			SIMU.ModuleData[i].szSourceDir );		
		
		SetWindowText( GetDlgItem( SIMU.ModuleData[i].hDlg, IDC_DESTINATION ),
			SIMU.ModuleData[i].szDestinationDir );		
	}

	// Create simple dialog
	SIMU.hDlg = CreateDialog( (HANDLE)SIMU.hInst,	// Instance
		MAKEINTRESOURCE(IDD_COMMAND),				// Dialog box template name
		SIMU.hWnd,									// Handle to parent window
		(DLGPROC)MainDlgProc);						// Dialog box procedure

	if(SIMU.hDlg == NULL)
	{// Failed to create dialog, abort
		return FALSE;
	}
	
	SIMU.hWndStatus = GetDlgItem( SIMU.hDlg, IDC_STATUS );
	
	SetWindowPos( SIMU.hDlg,
		HWND_NOTOPMOST,
		0, Top,
		0, 0, 
		SWP_NOZORDER|SWP_NOSIZE );		
	
	GetClientRect( SIMU.hDlg, &sRect );
	
	Top += sRect.bottom;
	
	ShowWindow( SIMU.hDlg, SW_SHOWNORMAL );	

	SetWindowPos( SIMU.hWnd, HWND_NOTOPMOST, 0, 0, sRect.right, Top+30, SWP_NOMOVE );

	// Disable start button untill subscribed to all module instances
	EnableWindow( GetDlgItem( SIMU.hDlg, IDSTART ), FALSE );

	// Get status icon handle
	SIMU.hWndStatusIcon = GetDlgItem( SIMU.hDlg, IDC_STATUS_ICON );

	SetStatus( APP_STATUS_INITIALIZING );

	return TRUE;
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
PRIVATE VOID HandleBrowse( HWND hDlg )
{
	BROWSEINFO
		sInfo;
	LPITEMIDLIST
		lpItemList;
	CHAR
		szTitle[MAX_PATH],
		szDirPath[MAX_PATH],
		szConfigKey[MAX_PATH],
		szLaneName[MAX_PATH];
	IMalloc
		*lpMalloc;
	LONG
		i,
		LaneNameLen;

	memset( &sInfo, 0, sizeof(BROWSEINFO) );
	
	// Replace with translation mechanism
	strcpy( szTitle, Translation( TXT_SELECT_DESTINATION ) );
	sInfo.lpszTitle = TEXT( szTitle );
	
	sInfo.hwndOwner			= hDlg;
	sInfo.pidlRoot			= NULL;
	sInfo.pszDisplayName	= NULL;
	sInfo.ulFlags			= 0;
	sInfo.lpfn				= NULL;
	sInfo.lParam			= (LPARAM)NULL;
	sInfo.iImage			= (INT)NULL;
	
	// Open window
	lpItemList = SHBrowseForFolder( &sInfo );
	if( lpItemList != NULL ) 
	{							
		SHGetPathFromIDList( lpItemList, szDirPath );

		for( i = 0; i < SIMU.NbModuleInstances; i++ )
		{
			if( SIMU.ModuleData[i].hDlg != hDlg )
			{
				continue;
			}

			// Get plaza name
			sprintf( szConfigKey, "%s%s%s%s",
				CSR_REG_KEYn_CSRBASE,
				CSR_REG_KEYn_LANE_BASE,
				CSR_REG_KEYn_PROJECT,
				"Parameters\\GeneralData\\" );
			
			LaneNameLen = MAX_PATH;
			
			if( REG_Lire_Chaine( CSR_REG_KEYi_ROOT,
									szConfigKey,
									"LaneName",
									szLaneName,
									&LaneNameLen ) != ERROR_SUCCESS )
			{
				strcpy( szLaneName, "Unknown" );
			}

			// Create destination path with lane name
			_makepath( SIMU.ModuleData[i].szDestinationDir,	NULL, szDirPath, szLaneName, NULL );
			
			SetWindowText( GetDlgItem( SIMU.ModuleData[i].hDlg, IDC_DESTINATION ),
				SIMU.ModuleData[i].szDestinationDir );

			SIMU.ModuleData[i].TargetPathSet = TRUE;
			break;
		}
		
		SHGetMalloc( &lpMalloc );
		lpMalloc->lpVtbl->Free( lpMalloc, lpItemList );
		lpMalloc->lpVtbl->Release( lpMalloc );

		if( AllTargetPathsSet() != FALSE )
		{// Enable start button
			EnableWindow( GetDlgItem( SIMU.hDlg, IDSTART ), TRUE );
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
PRIVATE VOID HandleStart( VOID )
{
	LONG
		i;

	if( AllTargetPathsSet() != TRUE )
	{// Not all target paths are set, abort
		return;
	}
		
	// Disable browse buttons while working
	for( i = 0; i < SIMU.NbModuleInstances; i++ )
	{
		EnableWindow( GetDlgItem( SIMU.ModuleData[i].hDlg, IDC_BROWSE ), FALSE );
	}

	// Send message to module(s) to start exporting
	for( i = 0; i < SIMU.NbModuleInstances; i++ )
	{
		if( SIMU.ModuleData[i].SubscribedServices[M_EMI_PIC_MAINT_MODE] != TRUE )
		{// Not subscribed to MAINT MODE service on this instance of module
			continue;
		}

		SIMU.ModuleData[i].Completed = FALSE;
		
		strcpy( SIMU.message.u.srv_maint_mode.u.szTargetDir, SIMU.ModuleData[i].szDestinationDir );
		SimuEnvoi( SIMU.ModuleData[i].MailboxID, M_EMI_PIC_MAINT_MODE, M_EMI_PIC_MAINT_MODE_START );

	}

	// Disable start
	EnableWindow( GetDlgItem( SIMU.hDlg, IDSTART ), FALSE );
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
PRIVATE BOOL AllTargetPathsSet( VOID )
{
	LONG
		i;

	for( i = 0; i < SIMU.NbModuleInstances; i++ )
	{
		if( SIMU.ModuleData[i].TargetPathSet != TRUE )
		{
			return FALSE;
		}
	}

	return TRUE;
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
PUBLIC VOID SetStatus( enum_app_status enStatus )
{
	if( enStatus < FIRST_APP_STATUS || enStatus > LAST_APP_STATUS )
	{// Out of enumeration bounds, invalid input
		return;
	}

	SIMU.enAppStatus = enStatus;

	switch( enStatus )
	{
		case APP_STATUS_INITIALIZING:
		case APP_STATUS_WORKING:
			SetIcon( SIMU.hWndStatusIcon, ICON_WAITING );
			break;
		
		case APP_STATUS_READY:
		case APP_STATUS_COMPLETED:
			SetIcon( SIMU.hWndStatusIcon, ICON_OK );			
			break;
		default:
			break;		
	}


	UpdateStatusText();
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
PRIVATE VOID UpdateStatusText( VOID )
{

	switch( SIMU.enAppStatus )
	{
		case APP_STATUS_INITIALIZING:
			SetWindowText( SIMU.hWndStatus, Translation(TXT_INITIALIZING) );
			break;

		case APP_STATUS_READY:
			SetWindowText( SIMU.hWndStatus, Translation(TXT_READY) );
			break;
			
		case APP_STATUS_WORKING:
			SetWindowText( SIMU.hWndStatus, Translation(TXT_WORKING) );
			break;
			
		case APP_STATUS_COMPLETED:
			SetWindowText( SIMU.hWndStatus, Translation(TXT_COMPLETED) );
			break;
			
		default:
			break;
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
PUBLIC BOOL AllInstancesCompleted( VOID )
{
	LONG
		i;

	for( i = 0; i < SIMU.NbModuleInstances; i++ )
	{
		if( SIMU.ModuleData[i].Completed != TRUE )
		{
			return FALSE;
		}
	}

	return TRUE;
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
PUBLIC VOID ReportMaintComplete( VOID )
{
	LONG
		i;

	CHAR
		szMessage[MAX_PATH]	= { 0 },
		szBuffer[MAX_PATH];

	for( i = 0; i < SIMU.NbModuleInstances; i++ )
	{
		sprintf( szBuffer, "[%d] - %s", SIMU.ModuleData[i].NbFilesTransfered, Translation( SIMU.ModuleData[i].szTitleLabel ) );
		strcat( szMessage, szBuffer );

		if( i != (SIMU.NbModuleInstances - 1) )
		{// Add newline if this is not last moduls instance
			strcat( szMessage, "\n" );
		}
	}

	MessageBox( SIMU.hDlg, szMessage, Translation( TXT_FILES_TRANSFERED ), MB_OK );
}
/*---------------------------------- END FILE   -----------------------------*/