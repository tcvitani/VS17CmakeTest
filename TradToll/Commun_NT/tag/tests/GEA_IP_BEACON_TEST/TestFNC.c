/********************************************************************************************************/
/*																										*/
/*	Projet		: BEACON MANAGER																		*/
/*																										*/
/*	Composant	: BEACON MANAGER																		*/
/*																										*/
/*	Module		: TestFNC.c																				*/
/*																										*/
/*	Description	: Notification through callback functions.												*/
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
#include <conio.h>

#define __BCM_DLL__
#include "BeaconManager.h"

#define __BCM_IP__
//#define __BCM_SERIAL__

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

HANDLE	hEvtVST = NULL;
HANDLE	hEvtConnect = NULL;


//___ Prototypes des fonctions locales __________________________________________________________________

static ST_BCM_REG_PTR Init( DWORD *argVersion, ST_BCM_CONFIG *argConfig, BYTE *argBeaconID );
static BCM_ERR Terminate( ST_BCM_REG_PTR argBcmRegPtr );
static BCM_ERR StartTransaction( ST_BCM_REG_PTR argBcmRegPtr );
static BCM_ERR ProcessTransaction( ST_BCM_REG_PTR argBcmRegPtr );
static BOOL GetLibProc( void );
static void BCM_EXPORT Callback( ST_BCM_REG_PTR argBcmRegPtr, BCM_CALLBACK argTypeCB, DWORD argParam );
static void BCM_EXPORT Alarm( ST_BCM_REG_PTR argBcmRegPtr, BCM_ALARMS argAlarm, DWORD argState );


/********************************************************************************************************/

void main( void )
{
	DWORD			version;
	ST_BCM_REG_PTR	bcmRegPtr = NULL;
	ST_BCM_CONFIG	bcmConfig;
	BYTE			beaconID[BCM_SIZE_BEACONID];
	DWORD			dwWait;
	int				i;

	hEvtVST = CreateEvent( NULL, TRUE, FALSE, NULL );
	hEvtConnect = CreateEvent( NULL, TRUE, FALSE, NULL );

	// Initialise the BeaconManager
	bcmRegPtr = Init( &version, &bcmConfig, beaconID );

	if ( bcmRegPtr != NULL )
	{
		// Display the version of the library
		printf( "BeaconManager v%d.%d.%d\n", (BYTE)((version>>16) & 0xFF),
											 (BYTE)((version>>8) & 0xFF),
											 (BYTE)(version & 0xFF) );

		// Display the configuration of the beacon
		printf( "Beacon version : %s\n", bcmConfig.version );
		printf( "Beacon frequency : %d\n", bcmConfig.frequency );
		printf( "Beacon serial baudRate : %d\n", bcmConfig.baudRate );

		printf( "Beacon BeaconID : " );
		for( i = 0; i < BCM_SIZE_BEACONID; i++ )
			printf( "%02X ", beaconID[i] );
		printf( "\n" );

		// Start the transaction
		StartTransaction( bcmRegPtr );

		if ( isBST )
		{
			printf( "Put an OBE on the beacon...\n" );

			// Wait for the library to receive a VST
			dwWait = WaitForSingleObject( hEvtVST, INFINITE );
			if ( dwWait == WAIT_OBJECT_0 && isTrx )
			{
				// Process the transaction
				ProcessTransaction( bcmRegPtr );
			}
		}

		//_______________________
		// Terminating gracefully

		printf( "Terminating...\n" );
		Terminate( bcmRegPtr );
	}
	else
		printf( "Fail to load the BeaconManager.Dll\n" );

	CloseHandle( hEvtVST );
	CloseHandle( hEvtConnect );

	printf( "Press Enter to exit" );
	getchar();
	exit(0);
}


/*------------------------------------------------------------------------------------------------------*/
ST_BCM_REG_PTR Init( DWORD *argVersion, ST_BCM_CONFIG *argConfig, BYTE *argBeaconID )
{
	ST_BCM_REG_PTR	myBcmRegPtr;
	ST_BCM_STATE	myState;
	BCM_ERR			result;

	// Load the BeaconManager.Dll
	if ( !GetLibProc() )
		return( NULL );

	// Get the version of the Dll
	*argVersion = lpfnGetLibVersion();

	// Initialise the BeaconManager
#if defined __BCM_SERIAL__
	result = lpfnInitManagerFNC( &myBcmRegPtr, 1, NULL, 1, BCM_CFG_115200, BCM_Secondary,
											   3000, FALSE, Callback, Alarm );
	SetEvent( hEvtConnect );
#elif defined __BCM_IP__
	result = lpfnInitManagerFNC_IP( &myBcmRegPtr, 1, NULL, "192.168.0.27", 10001, BCM_Secondary,
											   3000, FALSE, Callback, Alarm );
#endif

	if ( result == BCM_NoErr )
	{
		// Wait for the connection
		if ( result == BCM_NoErr )
			WaitForSingleObject( hEvtConnect, INFINITE );

		// Get the version and the configuration of the beacon
		result = lpfnGetConfig( myBcmRegPtr, argConfig );
		if ( result != BCM_NoErr )
			printf( "Fail to get the configuration of the beacon (Err : %d)\n", result );

		// Get the beaconID of the beacon
		result = lpfnGetBeaconID( myBcmRegPtr, argBeaconID );
		if ( result != BCM_NoErr )
			printf( "Fail to get the beaconID of the beacon (Err : %d)\n", result );

		// Get the State of the beacon
		result = lpfnCheckState( myBcmRegPtr, &myState );
		if ( result == BCM_NoErr )
		{
			printf( "State : %d, Mode : %d, Trx : %d\n", myState.state, myState.mode, myState.trxInProgress );
		}
		else
			printf( "Fail to get the state of the beacon (Err : %d)\n", result );
	}
	else
		printf( "Fail to initialise the BeaconManager (Err : %d)\n", result );

	return( myBcmRegPtr );
}


/*------------------------------------------------------------------------------------------------------*/
BCM_ERR Terminate( ST_BCM_REG_PTR argBcmRegPtr )
{
	BYTE	buffer[256];
	DWORD	lgBuffer;
	BCM_ERR	result = BCM_NoErr;

	// If the emission BST is activated
	if ( isBST )
	{
		result = lpfnStopBST( argBcmRegPtr );
		if ( result != BCM_NoErr )
			printf( "Fail to stop the BST (Err : %d)\n", result );
		isBST = FALSE;
	}

	// If a transaction is in progress send a Close
	if ( isTrx || result == BCM_TrxInProgress )
	{
		result = lpfnSendCmd( argBcmRegPtr, cmdEventReport, sizeof(cmdEventReport),
										    buffer, &lgBuffer, sizeof(buffer), TRUE );
		if ( result != BCM_NoErr )
			printf( "Fail to close the transaction (Err : %d)\n", result );
		isTrx = FALSE;
	}
	
	// Set the beacon in Stopped Mode
	result = lpfnChangeMode( argBcmRegPtr, BCM_MOD_Stopped );
	if ( result != BCM_NoErr )
		printf( "Fail to set the operating mode to Stopped (Err : %d)\n", result );

	// Terminate the BeaconManager
	result = lpfnCloseManager( &argBcmRegPtr );
	if ( result != BCM_NoErr )
		printf( "Fail to terminate the BeaconManager (Err : %d)\n", result );

	if ( hDll != NULL )
		FreeLibrary( hDll );

	return( result );
}


/*------------------------------------------------------------------------------------------------------*/
BCM_ERR	StartTransaction( ST_BCM_REG_PTR argBcmRegPtr )
{
	BCM_ERR	result;

	// Set the beacon in Transparent mode
	result = lpfnChangeMode( argBcmRegPtr, BCM_MOD_Transparent );
	if ( result != BCM_NoErr )
		printf( "Fail to set the operating mode to Transparent (Err : %d)\n", result );

	// Start the BST
	result = lpfnStartBST( argBcmRegPtr, cmdBST, sizeof(cmdBST), BCM_BST_ChangeBID );
	if ( result != BCM_NoErr )
		printf( "Fail to start the BST (Err : %d)\n", result );
	else
		isBST = TRUE;

	return( result );
}


/*------------------------------------------------------------------------------------------------------*/
BCM_ERR ProcessTransaction( ST_BCM_REG_PTR argBcmRegPtr )
{
	BYTE	buffer[256];
	DWORD	lgBuffer;
	BCM_ERR	result;

	// Get the VST
	result = lpfnGetVST( argBcmRegPtr, buffer, &lgBuffer, sizeof(buffer));
	if ( result != BCM_NoErr )
		printf( "Fail to get the VST (Err : %d)\n", result );

	// Request an echo
	if ( result == BCM_NoErr )
	{
		result = lpfnSendCmd( argBcmRegPtr, cmdEcho, sizeof(cmdEcho),
											buffer, &lgBuffer, sizeof(buffer), FALSE );
		if ( result != BCM_NoErr )
			printf( "Fail to request an echo (Err : %d)\n", result );
	}

	// Request a SetMMI
	if ( result == BCM_NoErr )
	{
		result = lpfnSendCmd( argBcmRegPtr, cmdSetMMI, sizeof(cmdSetMMI),
											buffer, &lgBuffer, sizeof(buffer), FALSE );
		if ( result != BCM_NoErr )
			printf( "Fail to request a SetMMI (Err : %d)\n", result );
	}

	// Close the transaction
	if ( result == BCM_NoErr )
	{
		result = lpfnSendCmd( argBcmRegPtr, cmdEventReport, sizeof(cmdEventReport),
											buffer, &lgBuffer, sizeof(buffer), TRUE );
		if ( result != BCM_NoErr )
			printf( "Fail to close the transaction (Err : %d)\n", result );

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

	// Check that all functions are present in the Dll
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
void BCM_EXPORT Callback( ST_BCM_REG_PTR argBcmRegPtr, BCM_CALLBACK argTypeCB, DWORD argParam )
{
	switch( argTypeCB )
	{
		case BCM_CB_IN :
			printf( "### VST Received ###\n" );

			// Declare the transaction in progress
			isTrx = TRUE;
			break;

		case BCM_CB_ERR:
			printf( "### ERROR : %d ###\n", argParam );
			break;
	}

	// The emission of the BST is automatically stopped
	isBST = FALSE;

	SetEvent( hEvtVST );
}


/*------------------------------------------------------------------------------------------------------*/
void BCM_EXPORT Alarm( ST_BCM_REG_PTR argBcmRegPtr, BCM_ALARMS argAlarm, DWORD argState )
{
	static char *gLabel[] = {"","Beacon not ready","Beacon KO","Beacon reset","Beacon OK"};
	DWORD		regNum;
	VOID		*userParam;

	// Get the user parameters
	lpfnGetUserParams( argBcmRegPtr, &regNum, &userParam );

	// If it is an event
	if ( argAlarm > BCM_AlarmBeacon )
	{
		printf("### EVENT (Reg:%d, Param:%d - %s) ###\n", regNum, (DWORD)userParam, gLabel[argAlarm] );

		// If the beacon has reset, terminate
		if ( argAlarm == BCM_EventReset && isBST )
		{
			isBST = FALSE;
			SetEvent( hEvtVST );
		}
	}
	// If it is an alarm
	else
	{
		if ( argState )
			printf("### ALARM (Reg:%d, Param:%d - %s - ON) ###\n", regNum, (DWORD)userParam, gLabel[argAlarm]);
		else
			printf("### ALARM (Reg:%d, Param:%d - %s - OFF) ###\n", regNum, (DWORD)userParam, gLabel[argAlarm]);

		// Check for the connection
		if ( argAlarm == BCM_AlarmPeriph )
		{
			if ( argState )
				ResetEvent( hEvtConnect );
			else
				SetEvent( hEvtConnect );
		}
	}
}


/********************************************************************************************************/
