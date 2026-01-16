/* --------------------------------------------------------------------
 * (C) 2005 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CMFTP
 * FILE       : CMFTP.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : Commoc, ftp
 * --------------------------------------------------------------------
 * SUMMARY    : The ComMOCSvc extension for the FTP remote server.
 * --------------------------------------------------------------------
 * DESCRIPTION: This extension library supports the FTP file transfer
 *              between the local host and remote host. It contains 
 *              exported functions used by the ComMOCSvc service.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <wininet.h>

#include <trc.h>
#include <reg.h>
#include <csr_excpt.h>
#include <safe_winInet.h>

#define LOC_DEF
#include <cmftp.h>
#undef LOC_DEF

#include <cmhost.h>

#include <memclass.h>


#define FTP_DEFINITION "FTP://"


//
// Définition de données d'instance d'une connexion
//
typedef struct _HOST_INST
{
    // Handle de la trace
    TRC_EMETTEUR    hTrc;
	
	char            szHostName[MAX_PATH];
	char            szUserName[MAX_PATH];
	char            szPassword[MAX_PATH];
    DWORD           dwTransferType;
	DWORD           dwReties;
	DWORD           dwTimeout;
	DWORD           dwReceiveTimeout;
	DWORD			dwSessionInactivityTimeout;
	DWORD           dwFlags;
	HINTERNET       hSession;
	BOOL            bPassiveMode;
	
    // Nom du host
    char            szName[MAX_PATH];

	WATCH_DOG_THREAD_INST *pWdThreadInst;

	CRITICAL_SECTION cs;

}
    HOST_INST;


//
// PROTOTYPE DES FONCTIONS PRIVÉES
//





PRIVATE BOOL IsFtpPathDefinition( char * pcPath );

PRIVATE BOOL IsFilePathDefinition( char * pcPath );

PRIVATE BOOL ExtractPathFromDef( char * pcPathDef, char * pcPath, DWORD dwPathSize );

PRIVATE void StrTran( char * pcString, char cOriginal, char cTransform );

PRIVATE BOOL CreateInternetSession( HOST_INST * psInst, HINTERNET * hSession );

PRIVATE BOOL CreateInternetConnection( HOST_INST * psInst, HINTERNET * hConnection );

PRIVATE void CloseInternetConnection(HOST_INST * psInst, HINTERNET hConnection);
PRIVATE void CloseInternetSession( HOST_INST * psInst );
PRIVATE void SafeSetInternetSession( HOST_INST * psInst,  HINTERNET hSession);
PROTECTED HINTERNET SafeGetInternetSession( HOST_INST * psInst);

PROTECTED void SetInternetSession( HOST_INST * psInst,  HINTERNET hSession);
PROTECTED HINTERNET GetInternetSession( HOST_INST * psInst);
PROTECTED void HostTrace( HOST_INST * psInst, char * szFormat, ... );
PROTECTED void HostTraceDirect( HOST_INST * psInst, char * szFormat, ... );

PROTECTED	BOOL WaitThreadToExit(HANDLE hThrd, int iTimeout);
PROTECTED BOOL SafeCloseHandle(HINTERNET hHandle, int iTimeout);

//
// CODE DES FONCTIONS EXPORTEES
//




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT HANDLE WINAPI HostOpen( char * szName, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: szName     : The connection name.
 *             hKeyConfig : The handle to the registry key that contains
 *                          the connection configuration parameters.
 * --------------------------------------------------------------------
 * RETURN    : The connection handle if the connection is properly initialized.
 *             NULL if the initialization fails.
 * --------------------------------------------------------------------
 * ROLE      : It reads the module parameters used to establish the FTP connection.
 *             The function search for the next parameters (attentin, these
 *             parameters are mandatory):
 *
 *        * TraceFile (REG_SZ) : The trace file name.
 *
 *        * HostName (REG_SZ) : String that contains the FTP server name or 
 *          the FTP IP address.
 *
 *        * UserName (REG_SZ) : User name for the FTP server connection.
 *
 *        * Password (REG_SZ) : Password for the FTP server connection.
 *
 *        * TransferType (REG_DWORD) : FTP transfer type. If it is set to zero the 
 *          file will by binary transferred to the FTP server. Otherwise, it will 
 *          be transferred as ASCII file. It means if the ASCII transfer is enabled on
 *          the FTP server the file transferred between the UNIX system and Windows system
 *          will be converted.
 *
 *        * ConnectReties (REG_DWORD) : If a connection attempt still fails after the 
 *          specified number of tries, the request is cancelled
 *
 *        * ConnectTimeout (REG_DWORD) : Timeout in milliseconds. If a connection request
 *          takes longer than this time-out value, the request is cancelled. 
 *
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostOpen=_HostOpen@8" )
#endif
EXPORT HANDLE WINAPI HostOpen( char * szName, HKEY hKeyConfig )
{
    DWORD       dwErr = NO_ERROR;   // Code d'erreur à retourner
    DWORD       dwSize;             // Taille de buffer utilisée pour les accés registre
    HOST_INST * psInst = NULL;      // Pointe sur le bloc de données de l'instance en cours de création.
    char        szTrace[MAX_PATH];  // Fichier de trace
	DWORD		dwTrcMaxSize;

    //
    // DEFINITION D'UN ENSEMBLE DE MACROS SIMPLIFIANT L'ACCES AU REGISTRE
    //

#define LIRE_CHAINE(var,nom) \
    dwSize = sizeof(var);\
    dwErr = REG_Lire_Chaine( hKeyConfig, NULL, (nom), (var), &dwSize );\
    if ( dwErr != NO_ERROR )\
        __leave;\
    else

#define LIRE_CHAINE_LOG(var,nom) \
    dwSize = sizeof(var);\
    dwErr = REG_Lire_Chaine( hKeyConfig, NULL, (nom), (var), &dwSize );\
    if ( dwErr != NO_ERROR )\
    {\
        HostTrace( psInst, "%s / HostOpen / Echec lecture config %s", psInst->szName, (nom) );\
        __leave;\
    }\
    else

#define LIRE_ENTIER_LOG(var,nom) \
    dwErr = REG_Lire_Entier( hKeyConfig, NULL, (nom), &dwSize );\
    if ( dwErr != NO_ERROR )\
    {\
        HostTrace( psInst, "%s / HostOpen / Echec lecture config %s", psInst->szName, (nom) );\
        __leave;\
    }\
    (var) = dwSize

    __try
    {

        //
        // Allocation du bloc de données qui va contenir les informations
        // de l'instance de connexion RAS.
        //
        psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psInst) );
        if ( psInst == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        //
        // Mémoriser le nom du host
        //
		strncpy_s(psInst->szName, sizeof(psInst->szName), szName, _TRUNCATE);
        psInst->szName[sizeof(psInst->szName)-1] = 0;

        //
        // Initialiser la trace
        //
        LIRE_CHAINE( szTrace, "TraceFile" );
        if ( szTrace[0] != 0 )
        {
            dwErr = TRC_Initialise_Trace( 
                szName, 
                szTrace, 
                TRC_OPT_FICHIER | TRC_OPT_CONSOLE | TRC_OPT_NUMEROTATION,
                &psInst->hTrc );
            if ( dwErr != NO_ERROR )
            {
                __leave;
            }
			
			dwErr = REG_Lire_Entier( hKeyConfig, NULL, "FileMaxSize", &dwTrcMaxSize );
			if ( dwErr == NO_ERROR )
				TRC_Taille_Max_Fichier( psInst->hTrc, (LONGLONG)dwTrcMaxSize);
			else
				TRC_Taille_Max_Fichier( psInst->hTrc, (LONGLONG)2000000);

        }
        else
            psInst->hTrc = NULL;

        HostTrace( psInst, "%s / HostOpen / Début", psInst->szName );

        //
        // LIRE LES PARAMÈTRES DU HOST
        //
        LIRE_CHAINE_LOG( psInst->szHostName             , "HostName" );
        LIRE_CHAINE_LOG( psInst->szUserName             , "UserName" );
        LIRE_CHAINE_LOG( psInst->szPassword             , "Password" );
		LIRE_ENTIER_LOG( psInst->dwTransferType         , "TransferType" );
		LIRE_ENTIER_LOG( psInst->dwReties               , "ConnectReties" );
		LIRE_ENTIER_LOG( psInst->dwTimeout              , "ConnectTimeout" );
		LIRE_ENTIER_LOG( psInst->bPassiveMode           , "PassiveMode");
		

		dwErr = REG_Lire_Entier( hKeyConfig, NULL, "SessionInactivityTimeout", &dwSize );
		if ( dwErr != NO_ERROR )
		{
			psInst->dwSessionInactivityTimeout = 30000;
			HostTrace( psInst, "%s  HostOpen  Echec lecture config %s. Setting default to %u", 
				psInst->szName, "SessionInactivityTimeout",psInst->dwSessionInactivityTimeout);
		}
		else
			psInst->dwSessionInactivityTimeout = dwSize;


		if ( psInst->dwTransferType == 0) // Transfer file as binary
			psInst->dwFlags = INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE ;
		else // Transfer file as ASCII
			psInst->dwFlags = INTERNET_FLAG_TRANSFER_ASCII | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE ;

		
		InitializeCriticalSection(&psInst->cs);
		//Because of WinInet.dll Bug see MSDN Q176420: 
		//added to assure that InternetConnect will not block for undefined time 
		//use a little longer timer to let the normal wininet timeout be triggered   
		CreateWatchdogThread(psInst, &psInst->pWdThreadInst, psInst->dwTimeout + SAFE_WININET_RESONABLE_TIMEOUT, &psInst->cs, psInst->dwSessionInactivityTimeout); 
        
		//
        // Tout s'est bien passé.
        //
        dwErr = NO_ERROR;



        HostTrace( psInst, "%s / HostOpen / Fin", psInst->szName );


    }
    __finally
    {
        //
        // En cas d'erreur, et uniquement si le bloc des données de l'instance
        // a été alloué, on libère les resources allouées.
        //
        if ( ( dwErr != NO_ERROR ) && ( psInst != NULL ) )
        {
            if ( psInst->hTrc != NULL )
                TRC_Termine_Trace( psInst->hTrc );

            HeapFree( GetProcessHeap(), 0, psInst );
            psInst = NULL;
        }

        //
        // Finalement, mettre à jour le code d'erreur courant et retourner
        // le handle obtenu (ou NULL en cas d'erreur).
        //
        SetLastError( dwErr );
    }

	#undef LIRE_CHAINE
	#undef LIRE_CHAINE_LOG
	#undef LIRE_ENTIER_LOG

	return psInst;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostQueryStatus( HANDLE hCnx )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx : Handle returned from HostOpen
 * --------------------------------------------------------------------
 * RETURN    : TRUE if the FTP server is alive.
 * --------------------------------------------------------------------
 * ROLE      : Tests the FTP server status.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostQueryStatus=_HostQueryStatus@4" )
#endif
EXPORT BOOL   WINAPI HostQueryStatus( HANDLE hCnx )
{
	BOOL bStatus = FALSE;
	HOST_INST * psInst =  (HOST_INST*)hCnx;

    HINTERNET   hConnection = NULL;

	HostTrace( psInst, "HostQueryStatus ==> Start...");

	// Check if the FTP server is alive (open/close connection)
	bStatus = CreateInternetConnection( psInst, &hConnection );
	CloseInternetConnection(psInst, hConnection );

	HostTrace( psInst, "HostQueryStatus ==> %s FTP server status: %s", psInst->szHostName, ( bStatus ? "Alive" : "Down" ) );

    return bStatus;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void   WINAPI HostClose( HANDLE hCnx )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx : Handle returned from HostOpen
 * --------------------------------------------------------------------
 * RETURN    : Nothing
 * --------------------------------------------------------------------
 * ROLE      : Liberates the resources allocated for the module parameters.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostClose=_HostClose@4" )
#endif
EXPORT void   WINAPI HostClose( HANDLE hCnx )
{
    DWORD       dwErr = NO_ERROR;   // Code d'erreur à retourner
    HOST_INST * psInst;             // Bloc de données de l'instance

    //
    // Pointer au bon endroit.
    //
    psInst =  (HOST_INST*)hCnx;

	StopWatchdogThread(psInst->pWdThreadInst);

    HostTrace( psInst, "%s / HostClose / Début", psInst->szName );

	//
    // Libérer les ressources allouées pour la connexion
    //
    if ( psInst->hTrc != NULL ) TRC_Termine_Trace( psInst->hTrc );
	
	CloseInternetSession(psInst);

	DeleteCriticalSection(&psInst->cs);

    
	HeapFree( GetProcessHeap(), 0, psInst );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostPutFile( HANDLE hCnx, char * pcLocalPath , char * pcRemotePath )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle returned from HostOpen
 *             pcLocalPath : Local source file name (the path is included).
 *             pcRemotePath: Remote destination file name (the path is included).
 * --------------------------------------------------------------------
 * RETURN    : TRUE if the file is uploaded or FALSE if the upload fails.
 * --------------------------------------------------------------------
 * ROLE      : Performs the file upload.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostPutFile=_HostPutFile@12" )
#endif
EXPORT BOOL   WINAPI HostPutFile( HANDLE hCnx, char * pcLocalPath , char * pcRemotePath )
{
	char  szPathSrc[MAX_PATH] = {0};
	char  szPathDst[MAX_PATH] = {0};
	BOOL bResult;
    DWORD dwErr = NO_ERROR;
	HOST_INST * psInst =  (HOST_INST*)hCnx;

    HINTERNET   hConnection = NULL;

	ExtractPathFromDef(pcLocalPath, szPathSrc, sizeof(szPathSrc));
	ExtractPathFromDef(pcRemotePath, szPathDst, sizeof(szPathDst));

	__try
	{
		// Check the connection type (FTP or not)
		if ( IsFtpPathDefinition( pcLocalPath ) || IsFtpPathDefinition( pcRemotePath ) )
		{
			// If it is FTP connection
			if ( !CreateInternetConnection( psInst, &hConnection ) )
			{
				HostTrace( psInst, "HostPutFile ==> HostName:%s Error connecting!", psInst->szName );

				__leave;
			}	
		}

		// Determine the action
		if ( IsFtpPathDefinition( pcRemotePath ) &&  IsFilePathDefinition( pcLocalPath ) )
		{
			HostTrace( psInst, "HostMoveFile ==> Copy file from %s to %s (FTP transfer). START: FtpPutFile...", szPathSrc, szPathDst );
			bResult = FtpPutFile( hConnection, szPathSrc, szPathDst, psInst->dwFlags, 0);
			HostTrace( psInst, "HostMoveFile ==> END: FtpPutFile!");
		}
		else if ( IsFilePathDefinition( pcRemotePath ) &&  IsFtpPathDefinition( pcLocalPath ) )
		{
			HostTrace( psInst, "HostMoveFile ==> Copy file from %s to %s (FTP transfer). START:FtpGetFile...", szPathSrc, szPathDst );
			bResult = FtpGetFile(  hConnection, szPathSrc, szPathDst, FALSE, FILE_ATTRIBUTE_NORMAL, psInst->dwFlags, 0 );
			HostTrace( psInst, "HostMoveFile ==> END: FtpGetFile!");
		}
		else if ( IsFilePathDefinition( pcRemotePath ) &&  IsFilePathDefinition( pcLocalPath ) )
		{
			HostTrace( psInst, "HostMoveFile ==> Copy file from %s to %s. START:CopyFile...", szPathSrc, szPathDst );
			bResult = CopyFile( szPathSrc, szPathDst,  TRUE );
			HostTrace( psInst, "HostMoveFile ==> END: CopyFile!");
		}
		else
		{
			HostTrace( psInst, "HostPutFile ==> Wrong definition of local or remote directories" );
		}
	}
	__finally
	{
		if ( ! bResult )
		{
			dwErr = GetLastError();
			HostTrace( psInst, "HostPutFile ==> Error %u!",dwErr);
		}	
		else
			dwErr = NO_ERROR;

		CloseInternetConnection(psInst, hConnection);

		SetLastError( dwErr );
	}

	return bResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostGetFile( HANDLE hCnx, char * pcRemotePath, char * pcLocalPath )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle returned from HostOpen
 *             pcRemotePath: Remote destination file name (the path is included).
 *             pcLocalPath : Local source file name (the path is included).
 * --------------------------------------------------------------------
 * RETURN    : TRUE if the file is downloaded or FALSE if the download fails.
 * --------------------------------------------------------------------
 * ROLE      : Performs the file download.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostGetFile=_HostGetFile@12" )
#endif
EXPORT BOOL   WINAPI HostGetFile( HANDLE hCnx, char * pcRemotePath, char * pcLocalPath )
{
	char  szPathSrc[MAX_PATH] = { 0 };
	char  szPathDst[MAX_PATH] = { 0 };
	BOOL bResult;
    DWORD dwErr = NO_ERROR;
	HOST_INST * psInst =  (HOST_INST*)hCnx;

    HINTERNET   hConnection = NULL;

	ExtractPathFromDef(pcLocalPath, szPathSrc, sizeof(szPathSrc));
	ExtractPathFromDef(pcRemotePath, szPathDst, sizeof(szPathDst));

	__try
	{
		// Check the connection type (FTP or not)
		if ( IsFtpPathDefinition( pcLocalPath ) || IsFtpPathDefinition( pcRemotePath ) )
		{
			// If it is FTP connection
			if ( !CreateInternetConnection( psInst, &hConnection ) )
			{
				HostTrace( psInst, "HostGetFile ==> HostName:%s Error connecting!",psInst->szName);
				
				__leave;
			}	
		}

		// Determine the action
		if ( IsFtpPathDefinition( pcRemotePath ) &&  IsFilePathDefinition( pcLocalPath ) )
		{
			HostTrace( psInst, "HostGetFile ==> Copy file %s to %s (FTP file transfer).START: FtpGetFile...", szPathDst, szPathSrc );
			bResult = FtpGetFile(  hConnection, szPathDst, szPathSrc, FALSE, FILE_ATTRIBUTE_NORMAL, psInst->dwFlags, 0 );
			HostTrace( psInst, "HostGetFile ==> END: FtpGetFile!");
		}
		else if ( IsFilePathDefinition( pcRemotePath ) &&  IsFtpPathDefinition( pcLocalPath ) )
		{
			HostTrace( psInst, "HostGetFile ==> Copy file %s to %s (FTP file transfer).START:FtpPutFile...", szPathDst, szPathSrc );

			bResult = FtpPutFile( hConnection, szPathDst, szPathSrc, psInst->dwFlags, 0);
			HostTrace( psInst, "HostGetFile ==> END: FtpPutFile!");
		}
		else if ( IsFilePathDefinition( pcRemotePath ) &&  IsFilePathDefinition( pcLocalPath ) )
		{
			HostTrace( psInst, "HostGetFile ==> Copy file %s to %s. START: CopyFile...", szPathDst, szPathSrc );

			bResult = CopyFile( szPathDst, szPathSrc, TRUE );
			HostTrace( psInst, "HostGetFile ==> END: CopyFile!");
		}
		else
		{
			HostTrace( psInst, "HostGetFile ==> Wrong definition of local or remote directories" );
		}
	}
	__finally
	{
		if ( ! bResult )
		{	dwErr = GetLastError();
			HostTrace( psInst, "HostGetFile ==> Error %u!",dwErr);
		}
		else
			dwErr = NO_ERROR;

		CloseInternetConnection(psInst, hConnection);

		SetLastError( dwErr );
	}

	return bResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostMoveFile( HANDLE hCnx, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle returned from HostOpen
 *             pcRemoteSrc : Source file name.
 *             pcRemoteDst : Destination file name. If it is NULL, the file
 *                           will be deleted.
 *             bCopy       : If it is TRUE the file will be copied. Otherwise
 *                           it will be moved.
 * --------------------------------------------------------------------
 * RETURN    : TRUE if the file is successfully moved or copied.
 * --------------------------------------------------------------------
 * ROLE      : It copies or moves the file from one remote directory to
 *             another remote directory.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostMoveFile=_HostMoveFile@16" )
#endif
EXPORT BOOL   WINAPI HostMoveFile( HANDLE hCnx, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy )
{
	char  szPathSrc[MAX_PATH] = { 0 };
	char  szPathDst[MAX_PATH] = { 0 };
	BOOL bResult;
    DWORD dwErr = NO_ERROR;
	HOST_INST * psInst =  (HOST_INST*)hCnx;

    HINTERNET   hConnection = NULL;

	ExtractPathFromDef(pcRemoteSrc, szPathSrc, sizeof(szPathSrc));
	ExtractPathFromDef(pcRemoteDst, szPathDst, sizeof(szPathDst));

	__try
	{
		// Check the connection type (FTP or not)
		if ( IsFtpPathDefinition( pcRemoteSrc ) || IsFtpPathDefinition( pcRemoteDst ) )
		{
			// If it is FTP connection
			if ( !CreateInternetConnection( psInst, &hConnection ) )
			{
				HostTrace( psInst, "HostMoveFile ==> HostName:%s Error connecting!",psInst->szName);
				__leave;
			}	
		}

		// Determine the action
		if ( pcRemoteDst == NULL )
		{
			HostTrace( psInst, "HostMoveFile ==> Delete file %s",  szPathSrc );

			if ( IsFilePathDefinition( pcRemoteSrc ) )
				bResult = DeleteFile( szPathSrc );
			else
				bResult = FtpDeleteFile( hConnection, szPathSrc );

			HostTrace( psInst, "HostMoveFile ==> END: Delete file %s",  szPathSrc );

		}
		else if ( IsFtpPathDefinition( pcRemoteSrc ) &&  IsFtpPathDefinition( pcRemoteDst ) && !bCopy )
		{
			HostTrace( psInst, "HostMoveFile ==> Move file %s to %s (FTP move/rename file).START:FtpRenameFile...", szPathSrc, szPathDst );

			// Move and/or rename file between two FTP directories 
			bResult = FtpRenameFile( hConnection, szPathSrc, szPathDst );
			HostTrace( psInst, "HostMoveFile ==> END: FtpRenameFile!");
		}
		else if (IsFilePathDefinition( pcRemoteSrc ) &&  IsFilePathDefinition( pcRemoteDst ) && !bCopy)
		{
			HostTrace( psInst, "HostMoveFile ==> Move file from %s to %s. START: MoveFile...", szPathSrc, szPathDst );

			bResult = MoveFile( szPathSrc, szPathDst );
			HostTrace( psInst, "HostMoveFile ==> END: MoveFile!");
		}
		else if (IsFilePathDefinition( pcRemoteSrc ) &&  IsFilePathDefinition( pcRemoteDst ) && bCopy)
		{
			HostTrace( psInst, "HostMoveFile ==> Copy file from %s to %s. START: CopyFile...", szPathSrc, szPathDst );

			bResult = CopyFile( szPathSrc, szPathDst, TRUE );
			HostTrace( psInst, "HostMoveFile ==> END: CopyFile!");
		}
		else if ( IsFilePathDefinition( pcRemoteSrc ) &&  IsFtpPathDefinition( pcRemoteDst ) && bCopy )
		{
			HostTrace( psInst, "HostMoveFile ==> Copy file from %s to %s (FTP transfer).START: FtpPutFile...", szPathSrc, szPathDst );

			bResult = FtpPutFile( hConnection, szPathSrc, szPathDst, psInst->dwFlags, 0);
			HostTrace( psInst, "HostMoveFile ==> END: FtpPutFile!");
		}
		else if ( IsFtpPathDefinition( pcRemoteSrc ) &&  IsFilePathDefinition( pcRemoteDst ) && bCopy )
		{
			HostTrace( psInst, "HostMoveFile ==> Copy file from %s to %s (FTP transfer):START:FtpGetFile...", szPathSrc, szPathDst );

			bResult = FtpGetFile(  hConnection, szPathSrc, szPathDst, FALSE, FILE_ATTRIBUTE_NORMAL, psInst->dwFlags, 0 );
			HostTrace( psInst, "HostMoveFile ==> END: FtpGetFile!");
		}
		else
		{
			HostTrace( psInst, "HostMoveFile ==> Wrong definition of local or remote directories" );
		}
	}
	__finally
	{
		if ( ! bResult )
		{	
			dwErr = GetLastError();
			HostTrace( psInst, "HostMoveFile ==> Error %u!",dwErr);
		}
		else
			dwErr = NO_ERROR;

		CloseInternetConnection(psInst, hConnection);

		SetLastError( dwErr );
	}

	return bResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostEnumFiles( HANDLE hCnx, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle returned from HostOpen
 *             pcRemoteMask: File mask used for file enumeration.
 *             ppsFound    : If it is successful it returns the pointer to table of
 *                           WIN32_FIND_DATA structures that contains the information
 *                           about enumerated files.
 *             pdwCount    : If it is successful it returns number of enumerated files.
 * --------------------------------------------------------------------
 * RETURN    : TRUE if the files are successfully enumerated.
 * --------------------------------------------------------------------
 * ROLE      : It enumerates the content of the remote directory.
 *             Attention, this function doesn't enumerate the local directories.
 *             The table allocated with this function has to be de-allocated
 *             inside the HostFreeEnum function.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostEnumFiles=_HostEnumFiles@16" )
#endif
EXPORT BOOL   WINAPI HostEnumFiles( HANDLE hCnx, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount )
{
	char  szRemoteMask[MAX_PATH] = { 0 };
	HOST_INST * psInst =  (HOST_INST*)hCnx;

    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwIndex;
    DWORD dwErr = NO_ERROR;
    DWORD dwAllocated = 0;
    DWORD dwFiles = 0;
    BOOL bResult;
    WIN32_FIND_DATA sFile;
    WIN32_FIND_DATA * psFiles = NULL;
    WIN32_FIND_DATA * psNewFiles;
	DWORD dwCurrentReqID;
    HINTERNET   hConnection = NULL;
	HINTERNET   hFtpFind = NULL;

	ExtractPathFromDef(pcRemoteMask, szRemoteMask, sizeof(szRemoteMask));

	__try
	{
		dwIndex = 0;

		// Check the connection type (FTP or not)
		if ( IsFtpPathDefinition( pcRemoteMask ) )
		{
			HostTrace( psInst, "HostEnumFiles ==> Calling CreateInternetConnection!");

			// If it is FTP connection
			if ( !CreateInternetConnection( psInst, &hConnection ) )
			{
				dwErr = GetLastError();
				HostTrace( psInst, "HostEnumFiles ==> CreateInternetConnection : Error %u!",dwErr);
				__leave;
			}

			HostTrace( psInst, "HostEnumFiles ==> START:FtpFindFirstFile: szRemoteMask: [%s]...!",szRemoteMask);

			dwCurrentReqID = AppendCheckIfConnectionBlocked(psInst->pWdThreadInst, SafeGetInternetSession(psInst)); 
				bResult = ( ( hFtpFind = FtpFindFirstFile( hConnection, szRemoteMask, &sFile, INTERNET_FLAG_RELOAD, 0 ) ) != NULL );
				
				dwErr = GetLastError();
			RemoveCheckIfConnectionBlocked(psInst->pWdThreadInst, dwCurrentReqID);
			
			HostTrace( psInst, "HostEnumFiles ==> END:FtpFindFirstFile!");
		}
		else
		{
			bResult = ( ( hFind = FindFirstFile( szRemoteMask, &sFile ) ) != INVALID_HANDLE_VALUE );
			dwErr = GetLastError();
		}

		while ( bResult )
		{
			if ( ( sFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
			{
				while ( dwIndex >= dwAllocated )
				{
					dwAllocated += HOST_LIST_INCREMENT;
					if ( psFiles == NULL )
						psNewFiles = HeapAlloc( GetProcessHeap(), 0, sizeof(psFiles[0]) * dwAllocated );
					else
						psNewFiles = HeapReAlloc( GetProcessHeap(), 0, psFiles, sizeof(psFiles[0]) * dwAllocated );
					if ( psNewFiles == NULL )
					{
						dwErr = ERROR_NOT_ENOUGH_MEMORY;
						__leave;
					}
					psFiles = psNewFiles;
				}
				psFiles[dwIndex] = sFile;
				dwIndex++;
			}

			if ( IsFtpPathDefinition( pcRemoteMask ) )
			{
				// If it is FTP connection
				dwCurrentReqID = AppendCheckIfConnectionBlocked(psInst->pWdThreadInst, SafeGetInternetSession(psInst)); 
					bResult = InternetFindNextFile( hFtpFind, &sFile );
					dwErr = GetLastError();
				RemoveCheckIfConnectionBlocked(psInst->pWdThreadInst, dwCurrentReqID);
			}
			else
			{
				bResult = FindNextFile( hFind, &sFile );
				dwErr = GetLastError();
			}
		}

		if ( ( dwErr != ERROR_NO_MORE_FILES  ) && 
			( dwErr != ERROR_FILE_NOT_FOUND ) &&
			( dwErr != NO_ERROR )
			)
			__leave;

		(*ppsFound) = psFiles;
		(*pdwCount) = dwIndex;
		psFiles = NULL;

		dwErr = NO_ERROR;

		HostTrace( psInst, "HostEnumFiles ==> %d enumerated files in directory %s",  dwIndex, szRemoteMask );
	}
	__finally
	{
		if ( psFiles != NULL )
			HeapFree( GetProcessHeap(), 0, psFiles );
		if ( hFind != INVALID_HANDLE_VALUE )
			FindClose( hFind );

		if ( hFtpFind != NULL )
			InternetCloseHandle( hFtpFind );

		CloseInternetConnection(psInst, hConnection);

		SetLastError( dwErr );
	}

	return (dwErr == NO_ERROR);
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void   WINAPI HostFreeEnum( WIN32_FIND_DATA * psFind )
 * --------------------------------------------------------------------
 * PARAMETERS: psFind : Pointer to table of enumerated files
 *                      created by HostEnumFiles.
 * --------------------------------------------------------------------
 * RETURN    : Nothing.
 * --------------------------------------------------------------------
 * ROLE      : De-allocates the resources allocated by HostEnumFiles.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostFreeEnum=_HostFreeEnum@4" )
#endif
EXPORT void   WINAPI HostFreeEnum( WIN32_FIND_DATA * psFind )
{
    //
    // Libérer le bloc alloué.
    //
    HeapFree( GetProcessHeap(), 0, psFind );
}




//
// CODE DES FONCTIONS PRIVEES
//




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void HostTrace( HOST_INST * psInst, char * szFormat, ... )
 * --------------------------------------------------------------------
 * PARAMETERS: psInst        : Pointer to the instance structure.
 *             szFormat, ... ; Like printf
 * --------------------------------------------------------------------
 * RETURN    : Nothing
 * --------------------------------------------------------------------
 * ROLE      : It writes the string into the trace file
 * --------------------------------------------------------------------
 */
PROTECTED void HostTrace( HOST_INST * psInst, char * szFormat, ... )
{
    va_list pMark;

    if ( psInst->hTrc != NULL )
    {
        va_start( pMark, szFormat );
        TRC_Trace_Texte_V( psInst->hTrc, TRC_OPT_MASK, szFormat, pMark );
    }
}

PROTECTED void HostTraceDirect( HOST_INST * psInst, char * szFormat, ... )
{
    va_list pMark;
	
    if ( psInst->hTrc != NULL )
    {
        va_start( pMark, szFormat );
        TRC_Direct_Trace_Texte_V( psInst->hTrc, TRC_OPT_MASK, szFormat, pMark );
    }
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL IsFtpPathDefinition( char * pcPath )
 * --------------------------------------------------------------------
 * PARAMETERS: pcPath        : Path definition.
 * --------------------------------------------------------------------
 * RETURN    : TRUE if the path definition is directory or file stored
 *             on the FTP server.
 * --------------------------------------------------------------------
 * ROLE      : It determines if the path definition represnts the directory
 *             or file stored on the FTP server. If so the extension module
 *             performs the FTP connection and the FTP file transfer.
 * --------------------------------------------------------------------
 */
PRIVATE BOOL IsFtpPathDefinition( char * pcPath )
{
	if ( pcPath == NULL )
		return FALSE;

	if ( strlen ( pcPath ) >= 6 && _strnicmp ( pcPath, FTP_DEFINITION, 6 ) == 0)
		return TRUE;

	return FALSE;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL IsFilePathDefinition( char * pcPath )
 * --------------------------------------------------------------------
 * PARAMETERS: pcPath        : Path definition.
 * --------------------------------------------------------------------
 * RETURN    : TRUE if the path definition is shared directory or shared
 *             file.
 * --------------------------------------------------------------------
 * ROLE      : It determines if the path definition represnts shared
 *             or local directory. If so the extension module
 *             performs the file copy or file move.
 * --------------------------------------------------------------------
 */
PRIVATE BOOL IsFilePathDefinition( char * pcPath )
{
	if ( strlen ( pcPath ) >=2 )
		if ( pcPath[1] == ':' )
			return TRUE;

	if ( strlen ( pcPath ) >=3 )
		if ( strncmp ( pcPath, "\\\\", 2 ) == 0)
			return TRUE;

	return FALSE;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL ExtractPathFromDef( char * pcPathDef, char * pcPath, DWORD dwPathSize  )
 * --------------------------------------------------------------------
 * PARAMETERS: pcPathDef       : Path definition.
 *             pcPath          : Path extracted from the path definition
 * --------------------------------------------------------------------
 * RETURN    : TRUE if the path is successfully extracted.
 * --------------------------------------------------------------------
 * ROLE      : It extracts the path from the path definition if it
 *             represents the directory or file stored on the FTP server.
 *             It also transforms backslash to slash sign for the FTP directory
 *             definition.
 * --------------------------------------------------------------------
 */
PRIVATE BOOL ExtractPathFromDef(char * pcPathDef, char * pcPath, DWORD dwPathSize)
{
	if ( pcPathDef == NULL )
	{
		*pcPath = '\0';

		return TRUE;
	}

	if ( IsFtpPathDefinition( pcPathDef ) )
	{
		strcpy_s(pcPath, dwPathSize, &pcPathDef[6]);
		StrTran( pcPath, '\\', '/' );
	}
	else if ( IsFilePathDefinition( pcPathDef ) )
	{
		// Local directory definition
		strcpy_s(pcPath, dwPathSize, pcPathDef);
		StrTran( pcPath, '/', '\\' );
	}
	else
	{
		*pcPath = '\0';

		return FALSE;
	}

	return TRUE;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void StrTran( char * pcString, char cOriginal, char cTransform )
 * --------------------------------------------------------------------
 * PARAMETERS: pcString        : String that has to be transformed.
 *             cOriginal       : Character that should be replaced if
 *                               it exists inside the string
 *             cTransform      : Character that replace every occurence
 *                               of cOriginal
 * --------------------------------------------------------------------
 * RETURN    : Nothing
 * --------------------------------------------------------------------
 * ROLE      : Searches a string of a second character expression,
 *             and then replaces each occurrence with a third character
 *             expression.
 * --------------------------------------------------------------------
 */
PRIVATE void StrTran( char * pcString, char cOriginal, char cTransform )
{
	DWORD dwCounter;

	for ( dwCounter = 0; dwCounter < strlen( pcString ); dwCounter ++ )
	{
		if ( pcString[dwCounter] == cOriginal )
			pcString[dwCounter] = cTransform;
	}
}



PRIVATE const char * getNameForStatus(DWORD status)
{
	switch(status)
	{
	case INTERNET_STATUS_RESOLVING_NAME:	return "INTERNET_STATUS_RESOLVING_NAME";
	case INTERNET_STATUS_NAME_RESOLVED: return "INTERNET_STATUS_NAME_RESOLVED";   
	case INTERNET_STATUS_CONNECTING_TO_SERVER: return "INTERNET_STATUS_CONNECTING_TO_SERVER";   
	case INTERNET_STATUS_CONNECTED_TO_SERVER: return "INTERNET_STATUS_CONNECTED_TO_SERVER";    
	case INTERNET_STATUS_SENDING_REQUEST: return "INTERNET_STATUS_SENDING_REQUEST";        
	case INTERNET_STATUS_REQUEST_SENT: return "INTERNET_STATUS_REQUEST_SENT";			
	case INTERNET_STATUS_RECEIVING_RESPONSE: return "INTERNET_STATUS_RECEIVING_RESPONSE";     
	case INTERNET_STATUS_RESPONSE_RECEIVED: return "INTERNET_STATUS_RESPONSE_RECEIVED";       
	case INTERNET_STATUS_CTL_RESPONSE_RECEIVED: return "INTERNET_STATUS_CTL_RESPONSE_RECEIVED";   
	case INTERNET_STATUS_PREFETCH: return "INTERNET_STATUS_PREFETCH";              
	case INTERNET_STATUS_CLOSING_CONNECTION: return "INTERNET_STATUS_CLOSING_CONNECTION";    
	case INTERNET_STATUS_CONNECTION_CLOSED: return "INTERNET_STATUS_CONNECTION_CLOSED";      
	case INTERNET_STATUS_HANDLE_CREATED: return "INTERNET_STATUS_HANDLE_CREATED";       
	case INTERNET_STATUS_HANDLE_CLOSING: return "INTERNET_STATUS_HANDLE_CLOSING";       
	case INTERNET_STATUS_DETECTING_PROXY: return "INTERNET_STATUS_HANDLE_CLOSING";      
	case INTERNET_STATUS_REQUEST_COMPLETE: return "INTERNET_STATUS_REQUEST_COMPLETE";      
	case INTERNET_STATUS_REDIRECT: return "INTERNET_STATUS_REDIRECT";               
	case INTERNET_STATUS_INTERMEDIATE_RESPONSE: return "INTERNET_STATUS_INTERMEDIATE_RESPONSE";   
	case INTERNET_STATUS_USER_INPUT_REQUIRED: return "INTERNET_STATUS_USER_INPUT_REQUIRED";     
	case INTERNET_STATUS_STATE_CHANGE: return "INTERNET_STATUS_STATE_CHANGE";		
	case INTERNET_STATUS_COOKIE_SENT: return "INTERNET_STATUS_COOKIE_SENT";           
	case INTERNET_STATUS_COOKIE_RECEIVED: return "INTERNET_STATUS_COOKIE_RECEIVED";        
	case INTERNET_STATUS_PRIVACY_IMPACTED: return "INTERNET_STATUS_PRIVACY_IMPACTED";      
	case INTERNET_STATUS_P3P_HEADER: return "INTERNET_STATUS_P3P_HEADER";           
	case INTERNET_STATUS_P3P_POLICYREF: return "INTERNET_STATUS_P3P_POLICYREF";          
	case INTERNET_STATUS_COOKIE_HISTORY: return "INTERNET_STATUS_COOKIE_HISTORY";        

	}

	return "UNKNOWN STATUS";
}

void CALLBACK StatusFunc(HINTERNET hInternet,
						 DWORD_PTR dwContext,
						 DWORD Status,
						 LPVOID StatusInfo,
						 DWORD StatusInfoSize)
{		
	HOST_INST * psInst = (HOST_INST *)dwContext;

	

	HostTrace( psInst, "StatusFunc: status:%u - %s",  Status , getNameForStatus(Status));
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL CreateInternetSession( HOST_INST * psInst, HINTERNET * hSession )
 * --------------------------------------------------------------------
 * PARAMETERS: psInst        : Pointer to the instance structure.
 *             hSession      : Internet session handle
 * --------------------------------------------------------------------
 * RETURN    : TRUE if the session is initialized
 * --------------------------------------------------------------------
 * ROLE      : Opens the internet session and sets the session parameters
 *             (connection timeout and connection retries).
 * --------------------------------------------------------------------
 */
PRIVATE BOOL CreateInternetSession( HOST_INST * psInst, HINTERNET * phSession )
{
	// If the proxy server is not used
	*phSession = InternetOpen( psInst->szUserName, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);


	if ( *phSession != NULL )
	{
		if ( InternetSetOption( *phSession, INTERNET_OPTION_CONNECT_RETRIES, &psInst->dwReties, 4 ) != TRUE )
		{
			InternetCloseHandle( *phSession );
			*phSession = NULL;
			HostTrace( psInst, "InternetSetOption: ERROR setting INTERNET_OPTION_CONNECT_RETRIES to %u",  psInst->dwReties );

			return FALSE;
		}

		if(InternetSetOption( *phSession, INTERNET_OPTION_CONNECT_TIMEOUT, &psInst->dwTimeout, 4 ) != TRUE )
		{
			InternetCloseHandle( *phSession );
		  	*phSession = NULL;
			HostTrace( psInst, "InternetSetOption: ERROR setting INTERNET_OPTION_CONNECT_TIMEOUT to %u",  psInst->dwTimeout );

			return FALSE;
		}


		if(InternetSetOption( *phSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &psInst->dwReceiveTimeout, 4 ) != TRUE )
		{
			InternetCloseHandle( *phSession );
			*phSession = NULL;
			HostTrace( psInst, "InternetSetOption: ERROR setting INTERNET_OPTION_RECEIVE_TIMEOUT to %u",  psInst->dwReceiveTimeout );
			
			return FALSE;
		}

		if(InternetSetOption( *phSession, INTERNET_OPTION_SEND_TIMEOUT, &psInst->dwReceiveTimeout, 4 ) != TRUE )
		{
			InternetCloseHandle( *phSession );
			*phSession = NULL;
			HostTrace( psInst, "InternetSetOption: ERROR setting INTERNET_OPTION_SEND_TIMEOUT to %u",  psInst->dwReceiveTimeout );
			
			return FALSE;
		}
		
			
		if(InternetSetOption( *phSession, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, &psInst->dwReceiveTimeout, 4 ) != TRUE )
		{
			InternetCloseHandle( *phSession );
			*phSession = NULL;
			HostTrace( psInst, "InternetSetOption: ERROR setting INTERNET_OPTION_DATA_RECEIVE_TIMEOUT to %u",  psInst->dwReceiveTimeout );
			
			return FALSE;
		}

		if(InternetSetOption( *phSession, INTERNET_OPTION_DATA_SEND_TIMEOUT, &psInst->dwReceiveTimeout, 4 ) != TRUE )
		{
			InternetCloseHandle( *phSession );
			*phSession = NULL;
			HostTrace( psInst, "InternetSetOption: ERROR setting INTERNET_OPTION_DATA_SEND_TIMEOUT to %u",  psInst->dwReceiveTimeout );
			
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	
	return TRUE;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL CreateInternetConnection( HOST_INST * psInst, HINTERNET * hSession, HINTERNET * hConnection )
 * --------------------------------------------------------------------
 * PARAMETERS: psInst        : Pointer to the instance structure.
 *             hSession      : Internet session handle
 *             hConnection   : Internet connection handle
 * --------------------------------------------------------------------
 * RETURN    : TRUE if FTP connection is successful
 * --------------------------------------------------------------------
 * ROLE      : Opens the FTP connection.
 * --------------------------------------------------------------------
 */
PRIVATE BOOL CreateInternetConnection(HOST_INST * psInst, HINTERNET * hConnection)
{
	DWORD dwErr;
	DWORD dwCurrentReqID;
	HINTERNET hSession;
	DWORD dwConnectFlags = INTERNET_FLAG_ASYNC | INTERNET_FLAG_DONT_CACHE;

	// Initialize Win32 Internet functions
	if (SafeGetInternetSession(psInst) == NULL)
		if (CreateInternetSession(psInst, &hSession))
			SafeSetInternetSession(psInst, hSession);


	HostTrace(psInst, "START: CreateInternetConnection...Connecting to: %s, using passive connection = %d", psInst->szHostName, psInst->bPassiveMode);

	dwCurrentReqID = AppendCheckIfConnectionBlocked(psInst->pWdThreadInst, SafeGetInternetSession(psInst));

	if (dwCurrentReqID > 0)
	{
		if (psInst->bPassiveMode)
			dwConnectFlags = INTERNET_FLAG_ASYNC | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PASSIVE;

		*hConnection = InternetConnect(SafeGetInternetSession(psInst),
			psInst->szHostName,
			INTERNET_DEFAULT_FTP_PORT,
			psInst->szUserName,
			psInst->szPassword,
			INTERNET_SERVICE_FTP,
			dwConnectFlags,
			(DWORD_PTR)psInst);
	}

	dwErr = GetLastError();
	
	RemoveCheckIfConnectionBlocked(psInst->pWdThreadInst, dwCurrentReqID);

	if ( *hConnection == NULL )
	{
		HostTrace( psInst, "CreateInternetConnection: Error %d connecting to %s FTP server", dwErr, psInst->szHostName );
		SetLastError( dwErr );

			//if not connected close the internet session
			CloseInternetSession(psInst);

		return FALSE;
	}
	else
	{
		InternetSetStatusCallback(*hConnection, (INTERNET_STATUS_CALLBACK)StatusFunc);
	}

	
	HostTrace( psInst, "END: CreateInternetConnection: Connected successfully to %s FTP server", psInst->szHostName );

	return TRUE;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void CloseInternetConnection( HINTERNET hConnection )
 * --------------------------------------------------------------------
 * PARAMETERS: hConnection   : Internet connection handle
 * --------------------------------------------------------------------
 * RETURN    : Nothing
 * --------------------------------------------------------------------
 * ROLE      : Closes the internet session and the internet connection if
 *             they were previously open.
 * --------------------------------------------------------------------
*/
PRIVATE void CloseInternetConnection(HOST_INST * psInst, HINTERNET hConnection )
{
	if ( hConnection != NULL )
	{
		InternetCloseHandle( hConnection );
	}

	CloseInternetSession(psInst);
}


PRIVATE void CloseInternetSession( HOST_INST * psInst)
{
	HINTERNET hCurrentInetSession = NULL;
	BOOL bHandleClosedOK;

	__try{
		EnterCriticalSection(&psInst->cs);
			hCurrentInetSession = GetInternetSession(psInst);
			
			if(hCurrentInetSession!=NULL)
			{
				HostTrace( psInst, "CloseInternetSession %u",hCurrentInetSession );
				
				bHandleClosedOK = SafeCloseHandle(hCurrentInetSession, SAFE_WININET_RESONABLE_TIMEOUT);
				
				HostTrace( psInst, "CloseInternetSession:: SafeCloseHandle - %s!", bHandleClosedOK?"OK":"NOK");
				
				SetInternetSession(psInst, NULL);
				
			}
	}
	__finally
	{
		LeaveCriticalSection(&psInst->cs);
	}

}
//-------------------------------------------------------------

PROTECTED void SetInternetSession( HOST_INST * psInst,  HINTERNET hSession)
{
	psInst->hSession = hSession;
}

PROTECTED HINTERNET GetInternetSession( HOST_INST * psInst)
{
	return psInst->hSession;
}

PRIVATE void SafeSetInternetSession( HOST_INST * psInst,  HINTERNET hSession)
{
	__try{
		EnterCriticalSection(&psInst->cs);
		
			psInst->hSession = hSession;
	
	}
	__finally
	{
		LeaveCriticalSection(&psInst->cs);
	}
}

PROTECTED HINTERNET SafeGetInternetSession( HOST_INST * psInst)
{
	HINTERNET hSession;
	__try{
		EnterCriticalSection(&psInst->cs);

		hSession = psInst->hSession;
	}
	__finally
	{
		LeaveCriticalSection(&psInst->cs);
	}
		
	return hSession;
}

//-----------------------------------------------------------------

PROTECTED	BOOL WaitThreadToExit(HANDLE hThrd, int iTimeout)
{
	BOOL bIsThreadAlive = TRUE;
	BOOL bOK = FALSE;
	DWORD dwExitCode;
	
	if (hThrd != NULL)
		bIsThreadAlive = TRUE;
	
	while(TRUE)
	{
		Sleep(10);
		
		if(GetExitCodeThread(hThrd,&dwExitCode))
		{
			if(dwExitCode==STILL_ACTIVE)
			{
				bIsThreadAlive = TRUE;
			}
			else if(dwExitCode == ERROR_SUCCESS)
			{
				bIsThreadAlive = FALSE;
				bOK = TRUE;
			}
			else 
			{
				bIsThreadAlive = TRUE;
				break;
			}	
		}
		
		if(!bIsThreadAlive )
		{
			break;
		}
		else if ((iTimeout -= 10) <= 0)
		{
			break;
		}
	}

	if(bIsThreadAlive)
		TerminateThread(hThrd, 0);
	
	return bOK;
}




PROTECTED BOOL SafeCloseHandleThread(HINTERNET *ph_Handle)
{
	HINTERNET hHandle = *ph_Handle;
	InternetCloseHandle(hHandle);
	
	return ERROR_SUCCESS;
}


PROTECTED BOOL SafeCloseHandle(HINTERNET h_Handle, int iTimeout)
{
	HINTERNET hHandleCopy;  
	HANDLE hThrd;
	DWORD dwIDThread;
	BOOL bRetOK = FALSE;
	
	if (h_Handle == 0)
		return FALSE;
	
	hHandleCopy = h_Handle;
	
	hThrd = CreateThread(NULL,  // no security attributes 
		0,                // use default stack size 
		(LPTHREAD_START_ROUTINE) SafeCloseHandleThread, 
		(LPVOID)&hHandleCopy, // param to thread func 
		CREATE_SUSPENDED, // creation flag 
		&dwIDThread);       // thread identifier 
	
	if(hThrd!=NULL)
	{
		ResumeThread(hThrd);	
		
		bRetOK = WaitThreadToExit(hThrd, iTimeout);

		CloseHandle(hThrd);
	}
	
	
	return bRetOK;
}
