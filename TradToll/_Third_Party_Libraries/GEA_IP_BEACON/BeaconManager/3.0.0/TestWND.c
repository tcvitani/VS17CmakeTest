/********************************************************************************************************/
/*																										*/
/*	Projet		: BEACON MANAGER																		*/
/*																										*/
/*	Composant	: BEACON MANAGER																		*/
/*																										*/
/*	Module		: TestWND.c																				*/
/*																										*/
/*	Description	: Notification through windows messages sent to a windows object.						*/
/*																										*/
/*	Auteur		: C.Salenc																				*/
/*																										*/
/*	Date modif	: 19/08/2010																			*/
/*																										*/
/********************************************************************************************************/
/*																										*/
/*	Copyright (c) 1998-2010 GEA Meylan France.	All rights reserved.									*/
/*																										*/
/********************************************************************************************************/


//___ #include __________________________________________________________________________________________

#include <windows.h>
#include <stdio.h>

#define __BCM_DLL__
#include "BeaconManager.h"

//#define __BCM_IP__
#define __BCM_SERIAL__


//___ #define ___________________________________________________________________________________________
//___ Prototypes ________________________________________________________________________________________
//___ Declarations des variables locales ________________________________________________________________

BYTE	cmdBST[15] = {0x91,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00};
BYTE	cmdEcho[9] = {0x91,0x05,0x00,0x0F,0x02,0x03,0xAA,0x55,0xAA};
BYTE	cmdSetMMI[6] = {0x91,0x05,0x00,0x0A,0x00,0x00};
BYTE	cmdEventReport[4] = {0x91,0x21,0x00,0x00};

HINSTANCE	hDll = NULL;

BCM_LPFN_GetLibVersion		lpfnGetLibVersion;
BCM_LPFN_InitManagerWND		lpfnInitManagerWND;
BCM_LPFN_InitManagerTHD		lpfnInitManagerTHD;
BCM_LPFN_InitManagerFNC		lpfnInitManagerFNC;
BCM_LPFN_InitManagerWND_IP	lpfnInitManagerWND_IP;
BCM_LPFN_InitManagerTHD_IP	lpfnInitManagerTHD_IP;
BCM_LPFN_InitManagerFNC_IP	lpfnInitManagerFNC_IP;
BCM_LPFN_CloseManager		lpfnCloseManager;
BCM_LPFN_ChangeMode			lpfnChangeMode;
BCM_LPFN_StartBST			lpfnStartBST;
BCM_LPFN_GetVST				lpfnGetVST;
BCM_LPFN_GetUserParams		lpfnGetUserParams;
BCM_LPFN_SendCmd			lpfnSendCmd;
BCM_LPFN_StopBST			lpfnStopBST;
BCM_LPFN_CheckState			lpfnCheckState;
BCM_LPFN_Reset				lpfnReset;
BCM_LPFN_SetConfig			lpfnSetConfig;
BCM_LPFN_GetConfig			lpfnGetConfig;
BCM_LPFN_GetBeaconID		lpfnGetBeaconID;

BOOL	isBST = FALSE;
BOOL	isTrx = FALSE;

BOOL	bConnected = FALSE;

int		cx, cy;
int		line = 8;


//___ Prototypes des fonctions locales __________________________________________________________________

static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
static ST_BCM_REG_PTR Init( HWND hWnd, DWORD *argVersion );
static BCM_ERR GetInfo( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr, ST_BCM_CONFIG *argConfig, BYTE *argBeaconID );
static BCM_ERR Terminate( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr );
static BCM_ERR StartTransaction( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr );
static BCM_ERR ProcessTransaction( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr );
static BOOL GetLibProc( void );
static void Callback( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr, WPARAM wParam, LPARAM lParam );
static void Alarm( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr, WPARAM wParam, LPARAM lParam );
static void Message( HWND hWnd, char *argStr, BCM_ERR argErr );


/********************************************************************************************************/

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	static char		szAppName[] = "BeaconManager Test2";

	WNDCLASS		wndClass;
	HWND			hWnd;
	MSG				msg;

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	wndClass.hbrBackground = (HBRUSH) GetStockObject( WHITE_BRUSH );
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = szAppName;
	
	if ( !RegisterClass( &wndClass ) )
	{
		MessageBox( NULL, "Fail to register windows class", szAppName, MB_ICONERROR );
		return( 0 );
	}

	hWnd = CreateWindow( szAppName, szAppName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
						 CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL );

	ShowWindow( hWnd, nShowCmd );
	UpdateWindow( hWnd );

	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return msg.wParam;
}


/********************************************************************************************************/
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static DWORD			version;
	static ST_BCM_REG_PTR	bcmRegPtr = NULL;
	static ST_BCM_CONFIG	bcmConfig;
	static BYTE				beaconID[BCM_SIZE_BEACONID];

	HDC				hDC;
	PAINTSTRUCT		ps;
	TEXTMETRIC		tm;
	char			strText[256];
	int				li, i;

	switch( message )
	{
		//______________
		case WM_CREATE :
			// Get some hDC stuff
			hDC = GetDC( hWnd );
			GetTextMetrics( hDC, &tm );
			cx = tm.tmAveCharWidth;
			cy = tm.tmHeight + tm.tmExternalLeading;
			ReleaseDC( hWnd, hDC );

			// Initialise the BeaconManager
			bcmRegPtr = Init( hWnd, &version );

			// Start the transaction
			if ( bcmRegPtr != NULL && bConnected )
			{
				GetInfo( hWnd, bcmRegPtr, &bcmConfig, beaconID );
				StartTransaction( hWnd, bcmRegPtr );
			}

			return( 0 );

		//______________
		case WM_PAINT :
			hDC = BeginPaint( hWnd, &ps );
			
			if ( bConnected )
			{
				li = 0;

				// Display the version of the library
				sprintf( strText, "BeaconManager v%d.%d.%d", (BYTE)((version>>16) & 0xFF),
															 (BYTE)((version>>8) & 0xFF),
															 (BYTE)(version & 0xFF) );
				TextOut( hDC, cx, (li++)*cy, strText, strlen(strText) );

				// Display the configuration of the beacon
				sprintf( strText, "Beacon version : %s", bcmConfig.version );
				TextOut( hDC, cx, (li++)*cy, strText, strlen(strText) );
				sprintf( strText, "Beacon frequency : %d", bcmConfig.frequency );
				TextOut( hDC, cx, (li++)*cy, strText, strlen(strText) );
				sprintf( strText, "Beacon serial baudRate : %d", bcmConfig.baudRate );
				TextOut( hDC, cx, (li++)*cy, strText, strlen(strText) );

				sprintf( strText, "Beacon BeaconID : " );
				for( i = 0; i < BCM_SIZE_BEACONID; i++ )
					sprintf( &strText[strlen(strText)], "%02X ", beaconID[i] );
				TextOut( hDC, cx, (li++)*cy, strText, strlen(strText) );

				if ( isBST )
				{
					li++;
					sprintf( strText, "Put an OBE on the beacon..." );
					TextOut( hDC, cx, (li++)*cy, strText, strlen(strText) );
				}
			}

			EndPaint( hWnd, &ps );
			return( 0 );

		//______________
		case WM_USER :
			Callback( hWnd, bcmRegPtr, wParam, lParam );
			return( 0 );

		//______________
		case WM_USER+1 :
			Alarm( hWnd, bcmRegPtr, wParam, lParam );

			// Start the transaction
			if ( bConnected )
			{
				GetInfo( hWnd, bcmRegPtr, &bcmConfig, beaconID );
				InvalidateRgn( hWnd, NULL, FALSE );
				UpdateWindow( hWnd );
				StartTransaction( hWnd, bcmRegPtr );
			}
			return( 0 );
		
		//______________
		case WM_DESTROY:
			// Terminating gracefully
			Terminate( hWnd, bcmRegPtr );

			PostQuitMessage( 0 );
			return( 0 );
	}

	return( DefWindowProc( hWnd, message, wParam, lParam ) );
}


/*------------------------------------------------------------------------------------------------------*/
ST_BCM_REG_PTR Init( HWND hWnd, DWORD *argVersion )
{
	ST_BCM_REG_PTR	myBcmRegPtr;
	BCM_ERR			result;

	// Load the BeaconManager.Dll
	if ( !GetLibProc() )
	{
		Message( hWnd, "Fail to load the BeaconManager.Dll", 0 );
		return( NULL );
	}

	// Get the version of the Library
	*argVersion = lpfnGetLibVersion();

	// Initialise the BeaconManager
#if defined __BCM_SERIAL__
	result = lpfnInitManagerWND( &myBcmRegPtr, 1, NULL, 1, BCM_CFG_115200, BCM_Secondary,
											   3000, FALSE, WM_USER, hWnd );
	bConnected = TRUE;

#elif defined __BCM_IP__
	result = lpfnInitManagerWND_IP( &myBcmRegPtr, 1, NULL, "133.38.40.157", 10001, BCM_Secondary,
											   3000, FALSE, WM_USER, hWnd );
#endif

	if ( result != BCM_NoErr )
		Message( hWnd, "Fail to initialise the BeaconManager (Err : %d)", result );
	
	return( myBcmRegPtr );
}


/*------------------------------------------------------------------------------------------------------*/
BCM_ERR GetInfo( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr, ST_BCM_CONFIG *argConfig, BYTE *argBeaconID )
{
	BCM_ERR	result;

	// Get the version and the configuration of the beacon
	result = lpfnGetConfig( argBcmRegPtr, argConfig );
	if ( result != BCM_NoErr )
		Message( hWnd, "Fail to get the configuration of the beacon (Err : %d)", result );

	// Get the beaconID of the beacon
	result = lpfnGetBeaconID( argBcmRegPtr, argBeaconID );
	if ( result != BCM_NoErr )
		Message( hWnd, "Fail to get the beaconID of the beacon (Err : %d)", result );

	return( result );
}


/*------------------------------------------------------------------------------------------------------*/
BCM_ERR Terminate( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr )
{
	BYTE	buffer[256];
	DWORD	lgBuffer;
	BCM_ERR	result = BCM_NoErr;

	// If the emission BST are activated
	if ( isBST )
	{
		result = lpfnStopBST( argBcmRegPtr );
		if ( result != BCM_NoErr )
			Message( hWnd, "Fail to stop the BST (Err : %d)", result );
		isBST = FALSE;
	}

	// If a transaction is in progress send a Close
	if ( isTrx || result == BCM_TrxInProgress )
	{
		result = lpfnSendCmd( argBcmRegPtr, cmdEventReport, sizeof(cmdEventReport),
										    buffer, &lgBuffer, sizeof(buffer), TRUE );
		if ( result != BCM_NoErr )
			Message( hWnd, "Fail to close the transaction (Err : %d)", result );
		isTrx = FALSE;
	}
	
	// Set the beacon in Stopped Mode
	result = lpfnChangeMode( argBcmRegPtr, BCM_MOD_Stopped );
	if ( result != BCM_NoErr )
		Message( hWnd, "Fail to set the operating mode to Stopped (Err : %d)", result );

	// Terminate the BeaconManager
	result = lpfnCloseManager( &argBcmRegPtr );
	if ( result != BCM_NoErr )
		Message( hWnd, "Fail to terminate the BeaconManager (Err : %d)", result );

	if ( hDll != NULL )
		FreeLibrary( hDll );

	return( result );
}


/*------------------------------------------------------------------------------------------------------*/
BCM_ERR	StartTransaction( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr )
{
	BCM_ERR	result;

	// Set the beacon in Transparent mode
	result = lpfnChangeMode( argBcmRegPtr, BCM_MOD_Transparent );
	if ( result != BCM_NoErr )
		Message( hWnd, "Fail to set the operating mode to Transparent (Err : %d)", result );

	// Start the BST
	result = lpfnStartBST( argBcmRegPtr, cmdBST, sizeof(cmdBST), BCM_BST_ChangeBID );
	if ( result != BCM_NoErr )
		Message( hWnd, "Fail to start the BST (Err : %d)", result );
	else
		isBST = TRUE;

	return( result );
}


/*------------------------------------------------------------------------------------------------------*/
BCM_ERR ProcessTransaction( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr )
{
	BYTE	buffer[256];
	DWORD	lgBuffer;
	BCM_ERR	result;

	// Declare the transaction in progress
	isTrx = TRUE;

	// Get the VST
	result = lpfnGetVST( argBcmRegPtr, buffer, &lgBuffer, sizeof(buffer));
	if ( result != BCM_NoErr )
		Message( hWnd, "Fail to get the VST (Err : %d)", result );

	// Request an echo
	if ( result == BCM_NoErr )
	{
		result = lpfnSendCmd( argBcmRegPtr, cmdEcho, sizeof(cmdEcho),
										   buffer, &lgBuffer, sizeof(buffer), FALSE );
		if ( result != BCM_NoErr )
			Message( hWnd, "Fail to request an echo (Err : %d)", result );
	}

	// Request a SetMMI
	if ( result == BCM_NoErr )
	{
		result = lpfnSendCmd( argBcmRegPtr, cmdSetMMI, sizeof(cmdSetMMI),
										   buffer, &lgBuffer, sizeof(buffer), FALSE );
		if ( result != BCM_NoErr )
			Message( hWnd, "Fail to request a SetMMI (Err : %d)", result );
	}

	// Close the transaction
	if ( result == BCM_NoErr )
	{
		result = lpfnSendCmd( argBcmRegPtr, cmdEventReport, sizeof(cmdEventReport),
										   buffer, &lgBuffer, sizeof(buffer), TRUE );
		if ( result != BCM_NoErr )
			Message( hWnd, "Fail to close the transaction (Err : %d)", result );

		// Declare the transaction closed
		isTrx = FALSE;
	}

	return( result );
}


/*------------------------------------------------------------------------------------------------------*/
BOOL GetLibProc( void )
{
	hDll = LoadLibrary( "BeaconManager.Dll" );
	if ( hDll == NULL )
		return( FALSE );

	// Get all the library functions
	lpfnGetLibVersion =		(BCM_LPFN_GetLibVersion)		GetProcAddress( hDll, BCM_FN_GetLibVersion );
	lpfnInitManagerWND =	(BCM_LPFN_InitManagerWND)		GetProcAddress( hDll, BCM_FN_InitManagerWND );
	lpfnInitManagerTHD =	(BCM_LPFN_InitManagerTHD)		GetProcAddress( hDll, BCM_FN_InitManagerTHD );
	lpfnInitManagerFNC =	(BCM_LPFN_InitManagerFNC)		GetProcAddress( hDll, BCM_FN_InitManagerFNC );
	lpfnInitManagerWND_IP =	(BCM_LPFN_InitManagerWND_IP)	GetProcAddress( hDll, BCM_FN_InitManagerWND_IP );
	lpfnInitManagerTHD_IP =	(BCM_LPFN_InitManagerTHD_IP)	GetProcAddress( hDll, BCM_FN_InitManagerTHD_IP );
	lpfnInitManagerFNC_IP =	(BCM_LPFN_InitManagerFNC_IP)	GetProcAddress( hDll, BCM_FN_InitManagerFNC_IP );
	lpfnCloseManager =		(BCM_LPFN_CloseManager)			GetProcAddress( hDll, BCM_FN_CloseManager );
	lpfnChangeMode =		(BCM_LPFN_ChangeMode)			GetProcAddress( hDll, BCM_FN_ChangeMode );
	lpfnStartBST =			(BCM_LPFN_StartBST)				GetProcAddress( hDll, BCM_FN_StartBST );
	lpfnGetVST =			(BCM_LPFN_GetVST)				GetProcAddress( hDll, BCM_FN_GetVST );
	lpfnGetUserParams =		(BCM_LPFN_GetUserParams)		GetProcAddress( hDll, BCM_FN_GetUserParams );
	lpfnSendCmd =			(BCM_LPFN_SendCmd)				GetProcAddress( hDll, BCM_FN_SendCmd );
	lpfnStopBST =			(BCM_LPFN_StopBST)				GetProcAddress( hDll, BCM_FN_StopBST );
	lpfnCheckState =		(BCM_LPFN_CheckState)			GetProcAddress( hDll, BCM_FN_CheckState );
	lpfnReset =				(BCM_LPFN_Reset)				GetProcAddress( hDll, BCM_FN_Reset );
	lpfnSetConfig =			(BCM_LPFN_SetConfig)			GetProcAddress( hDll, BCM_FN_SetConfig );
	lpfnGetConfig =			(BCM_LPFN_GetConfig)			GetProcAddress( hDll, BCM_FN_GetConfig );
	lpfnGetBeaconID =		(BCM_LPFN_GetBeaconID)			GetProcAddress( hDll, BCM_FN_GetBeaconID );

	// Check that all functions are present in the library
	if ( lpfnGetLibVersion == NULL ||
		 lpfnInitManagerWND == NULL ||
		 lpfnInitManagerTHD == NULL ||
		 lpfnInitManagerFNC == NULL ||
		 lpfnInitManagerWND_IP == NULL ||
		 lpfnInitManagerTHD_IP == NULL ||
		 lpfnInitManagerFNC_IP == NULL ||
		 lpfnCloseManager == NULL ||
		 lpfnChangeMode == NULL ||
		 lpfnStartBST == NULL ||
		 lpfnGetVST == NULL ||
		 lpfnGetUserParams == NULL ||
		 lpfnSendCmd == NULL ||
		 lpfnStopBST == NULL ||
		 lpfnCheckState == NULL ||
		 lpfnReset == NULL ||
		 lpfnSetConfig == NULL ||
		 lpfnGetConfig == NULL ||
		 lpfnGetBeaconID == NULL )

		return( FALSE );
	
	return( TRUE );
}


/*------------------------------------------------------------------------------------------------------*/
void Callback( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr, WPARAM wParam, LPARAM lParam )
{
	HDC			hDC;
	char		strText[256];

	hDC = GetDC( hWnd );

	switch( wParam )
	{
		case BCM_CB_IN :
			sprintf( strText, "### VST Received ###" );
			TextOut( hDC, cx, (line++)*cy, strText, strlen(strText) );
			
			// A transaction has started
			ProcessTransaction( hWnd, argBcmRegPtr );
			break;

		case BCM_CB_ERR:
			sprintf( strText, "### ERROR : %d ###", lParam );
			TextOut( hDC, cx, (line++)*cy, strText, strlen(strText) );
			break;
	}
	
	// The emission of the BST is automatically stopped
	isBST = FALSE;

	ReleaseDC( hWnd, hDC );
}


/*------------------------------------------------------------------------------------------------------*/
void Alarm( HWND hWnd, ST_BCM_REG_PTR argBcmRegPtr, WPARAM wParam, LPARAM lParam )
{
	static char *gLabel[] = {"","Beacon not ready","Beacon KO","Beacon reset","Beacon OK"};

	HDC			hDC;
	DWORD		regNum;
	VOID		*userParam;
	char		strText[256];

	hDC = GetDC( hWnd );

	// Get the user parameters
	lpfnGetUserParams( argBcmRegPtr, &regNum, &userParam );

	// If it is an event
	if ( wParam > BCM_AlarmBeacon )
	{
		sprintf( strText, "### EVENT (Reg:%d, Param:%d - %s) ###", regNum, userParam, gLabel[wParam] );
		TextOut( hDC, cx, (line++)*cy, strText, strlen(strText) );

		// If the beacon has reset, restart the transaction
		if ( wParam == BCM_EventReset && isBST )
			StartTransaction( hWnd, argBcmRegPtr );
	}
	// If it is an alarm
	else
	{
		if ( lParam )
		{
			sprintf( strText, "### ALARM (Reg:%d, Param:%d - %s - ON) ###", regNum, userParam, gLabel[wParam] );
			TextOut( hDC, cx, (line++)*cy, strText, strlen(strText) );
		}
		else
		{
			sprintf( strText, "### ALARM (Reg:%d, Param:%d - %s - OFF) ###", regNum, userParam, gLabel[wParam] );
			TextOut( hDC, cx, (line++)*cy, strText, strlen(strText) );
		}

		// Check for the connection
		if ( wParam == BCM_AlarmPeriph )
			bConnected = !lParam;
	}

	ReleaseDC( hWnd, hDC );
}


/*------------------------------------------------------------------------------------------------------*/
void Message( HWND hWnd, char *argStr, BCM_ERR argErr )
{
	char	strErr[256];

	sprintf( strErr, argStr, argErr );
	MessageBox( hWnd, strErr, "Error", MB_ICONERROR );
}


/********************************************************************************************************/
