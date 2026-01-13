/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : todbsvc
 * FILE       : todbsvc_bak.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Utilitaire de traitement des fichiers backup manuel
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <acom.h>
#include <ntsvc.h>
#include <dbif.h>
#include <col.h>
#include <csr_list.h>
#include <trc.h>

#include <todbsvc_glob.h>
#include <todbsvc_text.h>
#include <todbsvc_db.h>
#include <resource.h>

#include <memclass.h>

//#pragma warning (disable : 4996)


// --------------- CODE ----------------------


PRIVATE BOOL CALLBACK BakDlgProc(  
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam );

PRIVATE void BakDrop(  
        HWND hWnd,
        HANDLE hDrop,
        BOOL bDelete );

PRIVATE DWORD BakIncorporate( char * pcFile );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI BakMain( char * pcParams )
 * PARAMETERS: pcParams : Chaine contenant les paramètres de ligne de commande
 * RETURN    : NO_ERROR si success, sinon, un code Win32
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Traitement de l'appel du programme en ligne de commande
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI BakMain(
        char * pcParams )
{
    DWORD dwErr;    // Code d'erreur à retourner
    DWORD dwPos;    // Pour récupérer la position du paramètre erroné si erreur

    // Traitement ligne de commande
    if ( _stricmp( pcParams, "NULLPIPE" ) == 0 )
    {
        printf( "ToDBSvc - Null session pipe setup ...\n" );
        dwErr = AComSetNullSessionPipe( TODBSVC_PIPE_CMD, TRUE );
        if ( dwErr != NO_ERROR )
            printf( "ToDBSvc - Error %u\n", dwErr );
        else
            printf( "ToDBSvc - Null session pipe setup done\n" );
        return NO_ERROR;
    }

    ZeroMemory( &gsSvcWork, sizeof(gsSvcWork) );
    gsSvcWork.bIsCommand = TRUE;

    // Ouvrir la liste des paramètre du service (uniquement ceux dont on a besoin)
    printf( "ToDBSvc - Manual Backup File Management\n" );
    gsSvcWork.psParams = NTSVCOpenParameters( 
            TODBSVC_REG_VAL_MAXFILEMSGSIZE, REG_DWORD,        4,         16384, &gsSvcWork.sParmCopy.dwMaxFileMsgSize,
            TODBSVC_REG_VAL_DBUSR         , REG_SZ   , MAX_PATH,     "comuser", &gsSvcWork.sParmCopy.szDbUsr,
            TODBSVC_REG_VAL_DBPWD         , REG_SZ   , MAX_PATH,      "compwd", &gsSvcWork.sParmCopy.szDbPwd,
            TODBSVC_REG_VAL_DBINST        , REG_SZ   , MAX_PATH,         "pcs", &gsSvcWork.sParmCopy.szDbInst,
            NULL );

    // La liste a été ouverte
    if ( gsSvcWork.psParams != NULL )
    {
        // Récupérer les valeurs des paramètres
        dwErr = NTSVCLoadParameters( gsSvcWork.psParams, &dwPos );
        if ( dwErr == NO_ERROR )
        {
            // Tous les paramètres ont pu être récupéré
            gsSvcWork.sParmWork = gsSvcWork.sParmCopy;

            // Initialiser les requètes base
            DBInitRequests();
            
            // Si la liste des paramètres de ligne de commande est vies, alors ou est en
            // mode "boite de dialogue"
            if ( pcParams[0] == '\0' )
                dwErr = (DWORD)DialogBox( GetModuleHandle( NULL ), MAKEINTRESOURCE(IDD_TODBSVC), NULL, (DLGPROC)BakDlgProc );
            // Sinon, on est en mode console / ligne de commande
            else
                dwErr = BakIncorporate( pcParams );
        }
        else
        {
            printf( "ERROR : Cannot load parameters value for %s\n", gsSvcWork.psParams[dwPos].szName );
        }
    }
    else
    {
        printf( "ERROR : Cannot open parameters\n" );
        dwErr = ERROR_INVALID_PARAMETER;
    }
    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL CALLBACK BakDlgProc(  
 *                        HWND hWnd,
 *                        UINT uMsg,
 *                        WPARAM wParam,
 *                        LPARAM lParam )
 * PARAMETERS: Cf Win32 spec.
 * RETURN    : Cf Win32 spec.
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Callback procedure pour la boite de dialogue
 *             gérant le traitement des fichier par drag & drop.
 * --------------------------------------------------------------------
 */
PRIVATE BOOL CALLBACK BakDlgProc(  
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam )
{
    BOOL bProcessed = TRUE;     // Valeur booleenne à retourner
    BOOL bDelete;               // Indique si les fichiers traités avec succès doivent être effacés

    // ---------------------------------------
    if      ( uMsg == WM_INITDIALOG )
    {
        // Mettre en place la gestion du drad & drop sur la totalité de la boite de dialogue
        DragAcceptFiles( hWnd, TRUE );
    }

    // ---------------------------------------
    else if ( uMsg == WM_COMMAND )
    {
        // S'il s'agit du bouton "Close"
        if ( LOWORD(wParam) == IDCANCEL )
            PostQuitMessage( 0 );

        else
            bProcessed = FALSE;
    }
    
    // ---------------------------------------
    else if ( uMsg == WM_DROPFILES )
    {
        // Récupérer la valeur de la check box
        bDelete = ( SendDlgItemMessage( hWnd, IDC_DELETE_SUCCESS, BM_GETCHECK, 0, 0 ) == BST_CHECKED );
        // Traiter les fichiers dropés
        BakDrop( hWnd, (HANDLE)wParam, bDelete );
    }

    // ---------------------------------------
    else
        bProcessed = FALSE;

    return bProcessed;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void BakDrop(  
 *                        HWND hWnd,
 *                        HANDLE hDrop,
 *                        BOOL bDelete )
 * PARAMETERS: hWnd   : Handle de la boite de dialogue
 *             hDrop  : Handle de la liste des fichiers droppés
 *             bDelete: Si TRUE, les fichiers traités avec succès doivent être effacés
 *                      Si FALSE, ils sont conservés
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Traite les fichiers d'une liste droppée sur la boite de dialogue
 * --------------------------------------------------------------------
 */
PRIVATE void BakDrop(  
        HWND hWnd,
        HANDLE hDrop,
        BOOL bDelete )
{
    DWORD dwCount;          // Nombre de fichiers
    DWORD dwIndex;          // Pour scanner la liste des fichiers
    DWORD dwResult;         // Résultat de la gestion de la liste de fichiers
    DWORD dwErr;            // Code d'erreur avec interface db
    DWORD dwErrCount = 0;   // Nombre de fichiers en erreur
    DB_CNX * hDbCnx = NULL; // Handle de la connexion avec la base
    char szFile[MAX_PATH];  // Nom du fichier courant

    do
    {
        printf( "\nAnalysing dropped files" );
        // Déterminer le nombre de fichiers dans la liste
        if ( ( dwCount = DragQueryFile( hDrop, 0xFFFFFFFF, szFile, sizeof(szFile) ) ) == 0xFFFFFFFF )
        {
            printf( "\nERROR : Cannot handle dropped files\n" );
            break;
        }
        printf( "\r%u files have been dropped\n", dwCount );

        printf( "Connecting to the database" );
        // Effectuer la connexion à la base de données en utilisant les valeurs des
        // paramètres lues dans gsSvcWork.
        hDbCnx  = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
        gsSvcWork.bDatabaseMissing = ( hDbCnx == NULL );
        if ( hDbCnx == NULL )
        {
            printf( "\nERROR : Cannot connect to the database\n" );
            break;
        }
        printf( "\rThe connection to the database [%s] as [%s] is established\n", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr );

        for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
        {
            // Obtenir le nom du fichier correspondant à l'index
            dwResult = DragQueryFile( hDrop, dwIndex, szFile, sizeof(szFile) );
            if ( dwResult  != 0xFFFFFFFF )
            {
                printf( "FILE NUMBER %u : [%s] : ...", dwIndex + 1, szFile );
                // Alimenter la base avec les données du fichier
                dwErr = DBIncorporate( 
                    &hDbCnx, 
                    szFile, 
                    NULL, 
                    NULL, 
                    DB_FILE_BACKUP );
                if ( dwErr != NO_ERROR )
                {
                    // Ca n'a pas été accepté
                    dwErrCount ++;
                    printf( "\rFILE NUMBER %u : [%s] : Error %u\n", dwIndex + 1, szFile, dwErr );
                }
                else
                {
                    // Le fichier a été entièrement intégré
                    if ( bDelete )
                        // Si nécessaire, on l'efface
                        DeleteFile( szFile );
                    printf( "\rFILE NUMBER %u : [%s] : Success%s\n", dwIndex + 1, szFile, bDelete ?  " / File deleted" : "" );
                }
            }
            else
                printf( "FILE NUMBER %u : [] : Error, cannot retrieve file name\n", dwIndex + 1 );
        }
        printf( 
                "%s : %u/%u files have been successfully treated\n",
                dwErrCount == 0 ? "SUCCESS" : "ERROR",
                dwCount - dwErrCount,
                dwCount );
    }
    while ( FALSE );

    if ( hDbCnx != NULL )
        DBDisconnect( hDbCnx );

    DragFinish( hDrop );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD BakIncorporate( char * pcFile )
 * PARAMETERS: pcFile : Nom du fichier à traiter
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Traite un fichier passé en paramètre de ligne de commande
 * --------------------------------------------------------------------
 */
PRIVATE DWORD BakIncorporate( char * pcFile )
{
    DWORD dwErr;
    DB_CNX * hDbCnx = NULL;

    do
    {
        printf( "Connecting to the database" );
        // Effectuer la connexion à la base de données en utilisant les valeurs des
        // paramètres lues dans gsSvcWork.
        hDbCnx  = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
        gsSvcWork.bDatabaseMissing = ( hDbCnx == NULL );
        if ( hDbCnx == NULL )
        {
            printf( "\nERROR : Cannot connect to the database\n" );
            dwErr = ERROR_INVALID_DATA;
            break;
        }
        printf( "\rThe connection to the database [%s] as [%s] is established\n", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr );

        printf( "FILE [%s] : ...", pcFile );
        // Alimenter la base avec les données du fichier
        dwErr = DBIncorporate( &hDbCnx, pcFile, NULL, NULL, 0 );
        if ( dwErr != NO_ERROR )
            printf( "\rFILE [%s] : Error %u\n", pcFile, dwErr );
        else
            printf( "\rFILE [%s] : Success\n", pcFile );
    }
    while ( FALSE );

    if ( hDbCnx != NULL )
        DBDisconnect( hDbCnx );

    return dwErr;
}
