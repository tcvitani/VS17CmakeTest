/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : todbsvc
 * FILE       : todbsvc_db.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : base de données
 * --------------------------------------------------------------------
 * SUMMARY    : Module d'interface avec la base de données
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
#include <dbif.h>
#include <col.h>
#include <acom.h>
#include <ntsvc.h>
#include <csr_list.h>
#include <reg.h>
#include <trc.h>

#include <todbsvc_glob.h>
#include <todbsvc_text.h>
#include <todbsvc_lane.h>

#define LOC_DEF
#include <todbsvc_db.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)



#define DB_DEFAULT_BEGIN_LOAD \
       "DECLARE TrfMode CHAR(1); BEGIN IF :ContextId <> 0 THEN  NULL; END IF; IF :OriginId = 0 THEN  TrfMode := LOAD_MES.c_LOAD_ORIGINE_VOIE; ELSIF :OriginId = 1 THEN  TrfMode := LOAD_MES.c_LOAD_ORIGINE_BACKUP_DISK; ELSIF :OriginId = 2 THEN  TrfMode := LOAD_MES.c_LOAD_ORIGINE_BACKUP_TCO; ELSIF :OriginId = 3 THEN  TrfMode := LOAD_MES.c_LOAD_ORIGINE_NON_VOIE; ELSE  RAISE_APPLICATION_ERROR( -20001, 'OriginId>3' ); END IF; LOAD_MES.BLOAD_LANE_TRANSFERT( :PlazaId, :LaneId, TrfMode ); END;"
        
#define DB_DEFAULT_LOAD_MES \
       "BEGIN LOAD_MES.INSERT_MSG( :NbMsg, :Msg ); END;"

#define DB_DEFAULT_END_LOAD \
       "BEGIN IF :ErrorFlag <> 0 THEN  LOAD_MES.ELOAD_LANE_TRANSFERT( NULL, LOAD_MES.c_LOAD_ETAT_ERR ); ELSE  LOAD_MES.ELOAD_LANE_TRANSFERT(); END IF; END;"

#define DB_DEFAULT_LANE_LIST \
        "BEGIN IF :ContextId <> 0 THEN NULL; END IF; GEN_DEF.Get_Lane_LS( :PlazaId, :PlazaName, :LaneId, :LaneName); :TotalCount := 0; END;"

#define DB_DEFAULT_SET_LAST \
       "BEGIN  IF :ContextId <> 0 THEN  NULL; END IF; IF LOAD_MES.SET_LAST_SEQ_LANE( :PlazaId, :LaneId, :SeqNumber ) = 0 THEN  RAISE_APPLICATION_ERROR( -20001, 'SET_LAST_SEQ_LANE' ); END IF; END;"

#define DB_DEFAULT_GET_LAST \
       "BEGIN IF :ContextId <> 0 THEN  NULL; END IF; IF LOAD_MES.GET_LAST_SEQ_LANE( :PlazaId, :LaneId, :SeqNumber ) = 0 THEN  RAISE_APPLICATION_ERROR( -20001, 'GET_LAST_SEQ_LANE' ); END IF; END;"
                 
                

PRIVATE DWORD DBGetLanesFile();
PRIVATE DWORD DBSaveLanesFile();
PRIVATE char * DBTrim( char * pcStr );


PRIVATE char gszDbBeginLoadRequest[8000] = "";
PRIVATE char gszDbLoadMesRequest[8000]   = "";
PRIVATE char gszDbEndLoadRequest[8000]   = "";
PRIVATE char gszDbLaneListRequest[8000]  = "";
PRIVATE char gszDbSetLastRequest[8000]   = "";
PRIVATE char gszDbGetLastRequest[8000]   = "";



// --------------- CODE ----------------------


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD DBInitRequests()
 * PARAMETERS: Aucun
 * RETURN    : NO_ERROR si succès, sinon, un code Win32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Charge depuis le registre, les blocs SQL qui seront utilisés
 *             pour chacunes des actions en liaison avec la base
 * --------------------------------------------------------------------
 */
PROTECTED DWORD DBInitRequests()
{
    DWORD dwErr = NO_ERROR;
    DWORD dwLen;

#define LOAD_REQ(var,def) \
        dwLen = sizeof(gszDb##var); \
        dwErr = REG_Defaut_Chaine( \
            TODBSVC_REG_ROOT, \
            TODBSVC_REG_KEY_REQ, \
            #var, \
            gszDb##var, \
            &dwLen, \
            def ); \
        if ( dwErr != NO_ERROR ) \
            return dwErr;

    LOAD_REQ( BeginLoadRequest, DB_DEFAULT_BEGIN_LOAD )
    LOAD_REQ( LoadMesRequest  , DB_DEFAULT_LOAD_MES   )
    LOAD_REQ( EndLoadRequest  , DB_DEFAULT_END_LOAD   )
    LOAD_REQ( LaneListRequest , DB_DEFAULT_LANE_LIST  )
    LOAD_REQ( SetLastRequest  , DB_DEFAULT_SET_LAST   )
    LOAD_REQ( GetLastRequest  , DB_DEFAULT_GET_LAST   )

    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD DBGetLanes( )
 * PARAMETERS: Aucun
 * RETURN    : NO_ERROR si succès, sinon, un code Win32 ou Oracle
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Envoie un message de réponse sur la connexion de commande
 * --------------------------------------------------------------------
 */
PROTECTED DWORD DBGetLanes()
{
    DWORD dwErr = NO_ERROR;  // Code d'erreur à retourner
    DWORD dwCount;           // Stocker la taille des tables renvoyées
    DWORD dwReqCount;        // Taille des tables renvoyée par la base
    DWORD dwIndex;           // Index pour scanner les éléments des tables
    DB_VAR * hContextId;     // BIND : Handle du context
    DB_VAR * hPlazaId;       // BIND : Handle du tableau des id de gare
    DB_VAR * hPlazaName;     // BIND : Handle du tableau des noms de gare
    DB_VAR * hLaneId;        // BIND : Handle du tableau des id de voie
    DB_VAR * hLaneName;      // BIND : Handle du tableau des noms de voie
    DB_VAR * hTotalCount;    // BIND : Handle du nombre d'élement
    DB_STMT * hStmt;         // Handle de requete
    char szErr[200] = "";    // Texte d'erreur
    char szPlaza[TODBSVC_MAX_PLAZA_NAME] = ""; // Nom de gare
    char szLane[TODBSVC_MAX_PLAZA_NAME] = "";  // Nom de voie
    TODBSVC_LANE sLane;
    DB_CNX * hDbCnx;

    if ( gsSvcWork.sParmWork.dwLanesFileOnly == 0 )
    {
        // Boucle do while() utilisée pour la commodité du break. En faite, la condition
        // de bouclage est FALSE, on ne passe donc qu'une fois dedans.
        do 
        {
            // Essayer d'établir la connexion
            NTSVCInfo( "DBGetLanes(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr );
            hDbCnx  = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
            gsSvcWork.bDatabaseMissing = ( hDbCnx == NULL );
            // Si toujours pas connecté, on ne va pas plue long
            if ( hDbCnx == NULL )
            {
                dwErr = ERROR_PIPE_NOT_CONNECTED;
                NTSVCInfo( "DBGetLanes(), erreur %u, connexion à la base impossible", dwErr );
                break;
            }

            // REQUETE : OBTENIR LA LISTE DES VOIE POUR CE SYSTEME

            NTSVCInfo( "DBGetLanes(), préparation de la requête" );

            // Préparer la requête
            hStmt = DBOpenStatement( hDbCnx, gszDbLaneListRequest );
            if ( hStmt == NULL  )
            {
                DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, statement LaneListRequest : %s", dwErr, szErr );
                break;
            }

            // Associer les variables
            hContextId = DBBindPlaceHolderVariable( 
                    hStmt, 
                    ":ContextId", 
                    DB_TYPE_INT, 
                    sizeof(DWORD), 
                    0 );
            if ( hContextId == NULL )
            {
                DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind ContextId : %s", dwErr, szErr );
                break;
            }

            hPlazaId = DBBindPlaceHolderVariable( 
                    hStmt, 
                    ":PlazaId", 
                    DB_TYPE_INT, 
                    sizeof(DWORD), 
                    gsSvcWork.sParmWork.dwMaxLanes );
            if ( hPlazaId == NULL )
            {
                DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind PlazaId : %s", dwErr, szErr );
                break;
            }

            hPlazaName = DBBindPlaceHolderVariable( 
                    hStmt, 
                    ":PlazaName", 
                    DB_TYPE_STR, 
                    TODBSVC_MAX_PLAZA_NAME, 
                    gsSvcWork.sParmWork.dwMaxLanes );
            if ( hPlazaName == NULL )
            {
                DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind PlazaName : %s", dwErr, szErr );
                break;
            }

            hLaneId = DBBindPlaceHolderVariable( 
                    hStmt, 
                    ":LaneId", 
                    DB_TYPE_INT, 
                    sizeof(DWORD), 
                    gsSvcWork.sParmWork.dwMaxLanes );
            if ( hLaneId == NULL )
            {
                DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind LaneId : %s", dwErr, szErr );
                break;
            }

            hLaneName = DBBindPlaceHolderVariable( 
                    hStmt, 
                    ":LaneName", DB_TYPE_STR,
                    TODBSVC_MAX_LANE_NAME, 
                    gsSvcWork.sParmWork.dwMaxLanes );
            if ( hLaneName == NULL )
            {
                DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind LaneName : %s", dwErr, szErr );
                break;
            }

            hTotalCount = DBBindPlaceHolderVariable( 
                    hStmt, 
                    ":TotalCount", 
                    DB_TYPE_INT, 
                    sizeof(DWORD), 
                    0 );
            if ( hTotalCount == NULL )
            {
                DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind TotalCount : %s", dwErr, szErr );
                break;
            }

            // Initialiser les variables en input
            DBSetVariableItemValue( hContextId, 0, &gsSvcWork.sParmWork.dwContextId, 0 );

            NTSVCInfo( "DBGetLanes(), exécution de la requète" );

            // Exécuter la requète
            if ( ! DBExecuteStatement( hStmt ) )
            {
                DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, exécute LaneListRequest : %s", dwErr, szErr );
                break;
            }

            NTSVCInfo( "DBGetLanes(), Requète exécutée, début de l'analyse" );

            // Récupérer la taille des tableaux renvoyée par la base
            if ( DBGetVariableItemValue( hTotalCount, 0, &dwReqCount, NULL ) != &dwReqCount )
            {
                dwErr = ERROR_INVALID_DATA;
                NTSVCInfo( "DBGetLanes(), erreur %u, nombre d'éléments de table incohérent", dwErr );
                break;
            }

            // Récupérer la taille des résultats au cas ou la procedure
            // stockée a modifié la taille des tableaux (en principe, ce n'est pas le
            // cas) et vérifier que les tailles sont identiques pour tous les tableaux.
            // Dans tous les cas, plus loin, dwCount est ajusté en fonction de la validité des
            // items lus.
            dwCount = DBGetCurrentItemCount( hPlazaId );

            // S'assurer d'une cohérence des résultats
            if ( ( dwReqCount > 0 ) && ( dwReqCount < dwCount ) )
                dwCount = dwReqCount;

            if ( ( DBGetCurrentItemCount( hPlazaName ) < dwCount ) ||
                 ( DBGetCurrentItemCount( hLaneId )    < dwCount ) ||
                 ( DBGetCurrentItemCount( hLaneName )  < dwCount ) ||
                 ( dwCount > gsSvcWork.sParmWork.dwMaxLanes ) )
            {
                dwErr = ERROR_INVALID_DATA;
                NTSVCInfo( "DBGetLanes(), erreur %u, nombre d'éléments de table incohérent", dwErr );
                break;
            }

            ColLock( gsSvcWork.hLanes );

            // Mettre à zéro la table des nouvelles voies
            ColClear( gsSvcWork.hLanes );

            // Remplir la table avec les nouvelles valeurs
            for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
            {
                ZeroMemory( &sLane, sizeof(sLane ) );

                // Arrêter de parcourir si valeur non valide
                if ( DBGetVariableItemValue( hPlazaId, dwIndex, &sLane.sId.dwPlaza, NULL ) != &sLane.sId.dwPlaza )
                {
                    // Ajuster la le nombre des éléments lus
                    dwCount = dwIndex;
                    break;
                }
                if (  DBGetVariableItemValue( hLaneId, dwIndex, &sLane.sId.dwLane, NULL ) != &sLane.sId.dwLane )
                {
                    // Ajuster la le nombre des éléments lus
                    dwCount = dwIndex;
                    break;
                }
                if ( DBGetVariableItemValue( hPlazaName, dwIndex, szPlaza, NULL ) != szPlaza ) 
                    szPlaza[0] = '\0';
                if ( DBGetVariableItemValue( hLaneName, dwIndex, szLane, NULL ) != szLane ) 
                    szLane[0] = '\0';
                if ( ColItemAdd( gsSvcWork.hLanes, &sLane.sId, &sLane, sizeof(sLane ) ) == NULL )
                {
                    SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_COL_ADD" );
                    dwErr = ERROR_INVALID_DATA;
                }
                else
                {
                    NTSVCInfo( "DBGetLanes(), PZ%04u-LN%04u présente ([%s]-[%s])",
                                      sLane.sId.dwPlaza, 
                                      sLane.sId.dwLane,
                                      szPlaza,
                                      szLane );
                }
            }
            ColUnlock( gsSvcWork.hLanes );
            DBCloseStatement( hStmt );
            NTSVCInfo( "DBGetLanes(), %u voie sont enregistrées dans ce système", dwCount );
        }
        while ( FALSE );
    
        if ( hDbCnx != NULL )
        {
            NTSVCInfo( "DBGetLanes(), déconnection de la base" );
            DBDisconnect( hDbCnx );
            hDbCnx = NULL;
        }

        if ( dwErr != NO_ERROR )
        {
            // En cas d'erreur, on oublie la nouvelle liste
            ColClear( gsSvcWork.hLanes );
            if ( gsSvcWork.sParmWork.szLanesFile[0] != '\0' )
                dwErr = DBGetLanesFile();
        }
        else
        {
            if ( gsSvcWork.sParmWork.szLanesFile[0] != '\0' )
                DBSaveLanesFile();
        }
    }
    else
    {
        if ( gsSvcWork.sParmWork.szLanesFile[0] != '\0' )
            dwErr = DBGetLanesFile();
        else
            dwErr = ERROR_INVALID_PARAMETER;
    }

    return dwErr;
}


PROTECTED DWORD DBIncorporate( DB_CNX ** phDbCnx, char * pcFile, TODBSVC_LANE * psLane, DWORD * pdwMsgCount, DWORD dwMode )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwErr2;
    BOOL bVerify = ( ( dwMode & DB_FILE_VERIFY ) != 0 );
    BOOL bBackup = ( ( dwMode & DB_FILE_BACKUP ) != 0 );
    BOOL bLastTry = ( ( ( dwMode & DB_FILE_LAST_TRY ) != 0 ) || ( psLane == DB_NAL ) );
    HANDLE hFile = NULL;
    DWORD dwSize;
    DWORD dwRead;
    DWORD dwBuffer = 0;
    DWORD dwNewBuffer = 0;
    DWORD dwCount;
    BOOL bDeleteFile = FALSE;
    char * pcMsg = NULL;
    char * pcNewMsg = NULL;

    if ( pdwMsgCount != NULL )
        (*pdwMsgCount) = 0;

    // Boucle do{}while() utilisée par commodité (break), la condition du
    // while est FALSE, on ne passe donc qu'une fois dans la boucle
    do
    {
        if ( ! gsSvcWork.fLicenceIsValid )
        {
            dwErr = ERROR_INVALID_ACCESS;
            SVC_ERR_S( dwErr, "ERR_FILE_INVALID_LICENCE", pcFile );
            break;
        }

        // Si on est en phase d'incorporation et non de vérification, on doit appeler
        // la meme fonction en mode vérification avant de tenter l'incorporation.
        if ( ! bVerify )
        {
            if ( *phDbCnx == NULL )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

            // Appel récursif pour vérification de la structure du fichier
            dwErr = DBIncorporate( phDbCnx, pcFile, psLane, &dwCount, DB_FILE_VERIFY | ( bBackup ? DB_FILE_BACKUP : 0 ) );
            if ( dwErr != NO_ERROR )
            {
                SVC_ERR_S( dwErr, "ERR_FILE_INCORPORATE", pcFile );
                break;
            }

            // Si le fichier ne contient pas de message, inutile de continuer
            if ( dwCount == 0 )
            {
                NTSVCInfo( "DBIncorporate(), ficher sans message (ignoré) [%s]", pcFile );
                break;
            }

            // Signaler le début du chargement des messages en base.
            dwErr = DBBeginLoad( phDbCnx, psLane, bBackup );
            if ( dwErr != NO_ERROR )
            {
                SVC_ERR_S( dwErr, "ERR_FILE_BEGIN_INCORPORATE", pcFile );
                break;
            }
        }

        // Ouverture du fichier en lecture seule avec tout accès en exclusif
        hFile = CreateFile(
                pcFile,
                GENERIC_READ,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            dwErr = GetLastError();
            SVC_ERR_S( dwErr, "ERR_FILE_IO_INCORPORATE", pcFile );
            hFile = NULL;
            break;
        }

        // STRUCTURE DU FICHIER :
        // Le fichier à lire est constitué de séries de message. En tete de message,
        // on trouve une taille de type entier binaire codée sur 4 octets. C'est la taille
        // du message dont les données suivent immédiatement. Les messages sont tous à la
        // suite l'un de l'autre. La fin du fichier est marquée par un message de taille nulle.
        // CONDITIONS DE PURGE :
        // Le fichier ne doit être effacé qu'en cas de :
        //   - non validité de sa structure en phase de vérification.
        //   - non validité d'un message en phase d'incorporation
        // En cas de succes d'incorporation, le fichier est conservé
        do
        {
            // Lire la taille du prochain item (4 octets=sizeof(DWORD))
            if ( ! ReadFile( hFile, &dwSize, sizeof(dwSize), &dwRead, NULL ) )
            {
                dwErr = GetLastError();
                SVC_ERR_S( dwErr, "ERR_FILE_IO_INCORPORATE", pcFile );
                bDeleteFile = bVerify;
                break;
            }
            // L'i/o s'est bien déroulée, est-ce que le fichier était suffisament grand
            if ( dwRead != sizeof( dwSize ) )
            {
                dwErr = ERROR_INVALID_DATA;
                SVC_ERR_S( dwErr, "ERR_FILE_TRUNCATED_INCORPORATE", pcFile );
                bDeleteFile = bVerify;
                break;
            }
    
            // Taille nulle = fin de fichier
            if ( dwSize == 0 )
            {
                NTSVCInfo( "DBIncorporate(), fin du fichier [%s]", pcFile );
                bDeleteFile = FALSE;
                break;
            }

            // Taille de message trop grande
            if ( dwSize > gsSvcWork.sParmWork.dwMaxFileMsgSize )
            {
                dwErr = ERROR_INVALID_DATA;
                SVC_ERR_S( dwErr, "ERR_FILE_MSG_OVERSIZED_INCORPORATE", pcFile );
                bDeleteFile = bVerify;
                break;
            }

            // Si le buffer est trop petit, on le réajuste
            if ( dwSize > dwBuffer )
            {
                // On ajuste la taille du buffer en prenant le nombre minumum de
                // blocs de 1 KO pour faire tenir le message (avec le terminateur de chaine
                // qu'on doit rajouter)
                dwNewBuffer = ( ( ( dwSize + 1 ) / 1024 ) + 1 ) * 1024;

                // Si déjà alloué
                if ( dwBuffer > 0 )
                    pcNewMsg = HeapReAlloc( GetProcessHeap(), 0, pcMsg, dwNewBuffer );
                else
                    pcNewMsg = HeapAlloc( GetProcessHeap(), 0, dwNewBuffer );
                // Erreur d'allocation ou de réallocation
                if ( pcNewMsg == NULL )
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    // L'erreur ne provient pas du fichier, il ne faut pas l'effacer
                    bDeleteFile = FALSE; 
                    break;
                }
                pcMsg = pcNewMsg;
                dwBuffer = dwNewBuffer;
            }

            // Lire le message
            if ( ! ReadFile( hFile, pcMsg, dwSize, &dwRead, NULL ) )
            {
                dwErr = GetLastError();
                SVC_ERR_S( dwErr, "ERR_FILE_IO_INCORPORATE", pcFile );
                bDeleteFile = bVerify;
                break;
            }
            if ( dwRead != dwSize )
            {
                dwErr = ERROR_INVALID_DATA;
                SVC_ERR_S( dwErr, "ERR_FILE_TRUNCATED_INCORPORATE", pcFile );
                bDeleteFile = bVerify;
                break;
            }

            // Marquer la fin de chaine (et incrémenter la taille de buffer en conséquence
            pcMsg[dwSize ++] = '\0';

            // S'il s'agit d'une incorporation en base et non d'une vérification
            // de la structure du fichier
            if ( ! bVerify )
            {
                // Alimenter la base avec le message
                dwErr = DBInsertMessage( phDbCnx, pcMsg, dwSize );
                if ( dwErr != NO_ERROR )
                {
                    // Message non valide, on purge le fichier si ce n'est pas une erreur db
                    SVC_ERR_S( dwErr, "ERR_FILE_MSG_REFUSED_INCORPORATE", pcFile );
                    bDeleteFile = ( *phDbCnx != NULL );
                    break;
                }
            }
            if ( pdwMsgCount != NULL )
                (*pdwMsgCount) ++;
        }
        while ( dwErr == NO_ERROR );

        // S'il s'agit d'une incorporation en base et non d'une
        // vérification de la structure du fichier
        if ( ! bVerify )
        {
            // Signaler la fin du traitement
            // TO DO : Si on est en erreur, le ELOAD doit-il etre appelé avec des paramètres différents ?
            dwErr2 = DBEndLoad( phDbCnx, psLane, dwErr != NO_ERROR );
            if ( dwErr2 != NO_ERROR )
            {
                SVC_ERR_S( dwErr, "ERR_FILE_END_FAILED_INCORPORATE", pcFile );
                dwErr = ( dwErr == NO_ERROR ? dwErr2 : dwErr );
            }
        }
    }
    while ( FALSE );

    if ( hFile != NULL )
        CloseHandle( hFile );

    // Désallouer le buffer
    if ( pcMsg != NULL )
        HeapFree( GetProcessHeap(), 0, pcMsg );

    // Si le fichier doit être effacé, on le supprime
    // Ceci n'est possible que lorsque la voie est identifiée ou qu'il
    // s'agit d'un fichier NAL.
    if ( bDeleteFile && ( psLane != NULL ) )
    {
        if ( bLastTry && ( gsSvcWork.sParmWork.szRejectLaneDir[0] != '\0' ) )
        {
            char * pcName = strrchr( pcFile, '\\' );
            char szTrashFile[MAX_PATH*2];

            if ( pcName != NULL )
            {
                pcName ++;
                _snprintf_s( szTrashFile, _countof(szTrashFile), sizeof(szTrashFile), "%s\\%s", gsSvcWork.sParmWork.szRejectLaneDir, pcName );
                szTrashFile[sizeof(szTrashFile)-1] = '\0';
                
                bDeleteFile = ( ! MoveFileEx( pcFile, szTrashFile, MOVEFILE_REPLACE_EXISTING ) );

                if ( bDeleteFile )
                {
                    SVC_ERR_SS( dwErr, "ERR_FILE_TRASH_MV_ERR", pcFile, szTrashFile );
                }
                else
                {
                    SVC_ERR_SS( dwErr, "ERR_FILE_TRASH_MV_OK", pcFile, szTrashFile );
                }
            }
        }

        if ( bDeleteFile )
            DeleteFile( pcFile );
    }

    return dwErr;
}










PROTECTED DWORD DBBeginLoad( DB_CNX ** phDbCnx, TODBSVC_LANE * psLane, BOOL bBackup )
{
    char szErr[200] = "";    // Texte d'erreur
    DWORD dwErr = NO_ERROR;
    DWORD dwOrigin;
    DB_VAR * hContextId;     // BIND : Handle du context
    DB_VAR * hPlazaId;       // BIND : Handle de l'id de gare
    DB_VAR * hLaneId;        // BIND : Handle de l'id de voie
    DB_VAR * hOriginId;      // BIND : Handle de l'origine
    DB_STMT * hStmt = NULL;

    do
    {
        if ( *phDbCnx == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        // Préparer la requête
        NTSVCInfo( "DBBeginLoad(), préparation de la requête BeginLoadRequest" );
        if ( ( hStmt = DBOpenStatement( *phDbCnx, gszDbBeginLoadRequest ) ) == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_OPEN", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBOpenStatement:BeginLoadRequest) : %s\n", szErr );
            else
                NTSVCInfo( "DBBeginLoad(), erreur %u, statement BeginLoadRequest : %s", dwErr, szErr );
            break;
        }

        // Associer les variables
        hContextId = DBBindPlaceHolderVariable( 
                hStmt, 
                ":ContextId", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hContextId == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_BIND", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBBindPlaceHolderVariable:BeginLoadRequest) : %s\n", szErr );
            else
                NTSVCInfo( "DBBeginLoad(), erreur %u, Bind ContextId : %s", dwErr, szErr );
            break;
        }

        hPlazaId = DBBindPlaceHolderVariable( 
                hStmt, 
                ":PlazaId", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hPlazaId == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_BIND", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBBindPlaceHolderVariable:BeginLoadRequest) : %s\n", szErr );
            else
                NTSVCInfo( "DBBeginLoad(), erreur %u, Bind PlazaId : %s", dwErr, szErr );
            break;
        }

        hLaneId = DBBindPlaceHolderVariable( 
                hStmt, 
                ":LaneId", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hLaneId == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_BIND", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBBindPlaceHolderVariable:BeginLoadRequest) : %s\n", szErr );
            else
                NTSVCInfo( "DBBeginLoad(), erreur %u, Bind LaneId : %s", dwErr, szErr );
            break;
        }

        hOriginId = DBBindPlaceHolderVariable( 
                hStmt, 
                ":OriginId", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hOriginId == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_BIND", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBBindPlaceHolderVariable:BeginLoadRequest) : %s\n", szErr );
            else
                NTSVCInfo( "DBBeginLoad(), erreur %u, Bind OriginId : %s", dwErr, szErr );
            break;
        }

        // Initialiser les variables en input
        DBSetVariableItemValue( hContextId, 0, &gsSvcWork.sParmWork.dwContextId, 0 );
        if ( ( psLane != NULL ) && ( psLane != DB_NAL ) )
        {
            DBSetVariableItemValue( hPlazaId, 0, &psLane->sId.dwPlaza, 0 );
            DBSetVariableItemValue( hLaneId, 0, &psLane->sId.dwLane, 0 );
            dwOrigin = ( bBackup ? 2 : 0 );
        }
        else 
        {
            DBSetVariableItemValue( hPlazaId, 0, DB_VALUE_NULL, 0 );
            DBSetVariableItemValue( hLaneId, 0, DB_VALUE_NULL, 0 );
            dwOrigin = ( ( psLane != DB_NAL ) ? 1 : 3 );
        }
        DBSetVariableItemValue( hOriginId, 0, &dwOrigin, 0 );

        // Exécuter la requète
        NTSVCInfo( "DBBeginLoad(), exécution requête BeginLoadRequest" );
        if ( ! DBExecuteStatement( hStmt ) )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_EXEC", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBExecuteStatement:BeginLoadRequest): %s\n", szErr );
            else
                NTSVCInfo( "DBBeginLoad(), erreur %u, Execute BeginLoadRequest : %s", dwErr, szErr );
            break;
        }
        NTSVCInfo( "DBBeginLoad(), Fin de la requête BeginLoadRequest" );
    }
    while ( FALSE );
    
    if ( ( hStmt != NULL ) && ( *phDbCnx != NULL ) )
        DBCloseStatement( hStmt );

    return dwErr;
}




PROTECTED DWORD DBEndLoad( DB_CNX ** phDbCnx, TODBSVC_LANE * psLane, BOOL bErr )
{
    char szErr[200] = "";    // Texte d'erreur
    DWORD dwErr = NO_ERROR;
    DWORD dwErrorFlag = ( bErr ? 1 : 0 );
    DB_VAR * hErrorFlag;     // BIND : Handle de l'id de gare
    DB_STMT * hStmt = NULL;
 
    do
    {
        if ( *phDbCnx == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        // Préparer la requête
        NTSVCInfo( "DBEndLoad(), préparation de la requête EndLoadRequest" );
        hStmt = DBOpenStatement( 
                *phDbCnx,
                gszDbEndLoadRequest );
        if ( hStmt == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_OPEN", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBOpenStatement:EndLoadRequest): %s\n", szErr );
            else
                NTSVCInfo( "DBEndLoad(), erreur %u, Statement EndLoadRequest : %s", dwErr, szErr );
            break;
        }

        // Associer les variables
        hErrorFlag = DBBindPlaceHolderVariable( 
                hStmt, 
                ":ErrorFlag", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hErrorFlag == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_BIND", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBBindPlaceHolderVariable:EndLoadRequest) : %s\n", szErr );
            else
                NTSVCInfo( "DBEndLoad(), erreur %u, Bind ErrorFlag : %s", dwErr, szErr );
            break;
        }

        // Initialiser les variables en input
        DBSetVariableItemValue( hErrorFlag, 0, &dwErrorFlag, 0 );

        // Exécuter la requète
        NTSVCInfo( "DBEndLoad(), Execution de la requête EndLoadRequest" );
        if ( ! DBExecuteStatement( hStmt ) )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_EXEC", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBExecuteStatement:EndLoadRequest): %s\n", szErr );
            else
                NTSVCInfo( "DBEndLoad(), erreur %u, Execute EndLoadRequest : %s", dwErr, szErr );
            break;
        }
        NTSVCInfo( "DBEndLoad(), Fin de la requête EndLoadRequest" );
    }
    while ( FALSE );

    if ( ( hStmt != NULL ) && ( *phDbCnx != NULL ) )
        DBCloseStatement( hStmt );

    return dwErr;
}




PROTECTED DWORD DBSetLastTreatedNumber( DB_CNX ** phDbCnx, TODBSVC_LANE * psLane )
{
    char szErr[200] = "";    // Texte d'erreur
    DWORD dwErr = NO_ERROR;
    DB_VAR * hContextId;     // BIND : Handle du context
    DB_VAR * hPlazaId;       // BIND : Handle de l'id de gare
    DB_VAR * hLaneId;        // BIND : Handle de l'id de voie
    DB_VAR * hSeqNumber;     // BIND : Handle du numéro de séquence
    DB_STMT * hStmt;
 
    do
    {
        if ( *phDbCnx == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        // Préparer la requête
        NTSVCInfo( "DBSetLastTreatedNumber(), préparation de la requête SetLastRequest" );
        if ( ( hStmt = DBOpenStatement( *phDbCnx, gszDbSetLastRequest ) ) == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBSetLastTreatedNumber(), erreur %u, statement SetLastRequest : %s", dwErr, szErr );
            break;
        }

        // Associer les variables
        hContextId = DBBindPlaceHolderVariable(
                hStmt, 
                ":ContextId", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hContextId == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBSetLastTreatedNumber(), erreur %u, bind ContextId : %s", dwErr, szErr );
            break;
        }

        hPlazaId = DBBindPlaceHolderVariable(
                hStmt, 
                ":PlazaId", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hPlazaId == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBSetLastTreatedNumber(), erreur %u, bind PlazaId : %s", dwErr, szErr );
            break;
        }

        hLaneId = DBBindPlaceHolderVariable(
                hStmt, 
                ":LaneId", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hLaneId == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBSetLastTreatedNumber(), erreur %u, bind LaneId : %s", dwErr, szErr );
            break;
        }

        hSeqNumber = DBBindPlaceHolderVariable(
                hStmt, 
                ":SeqNumber", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hSeqNumber == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBSetLastTreatedNumber(), erreur %u, bind SeqNumber : %s", dwErr, szErr );
            break;
        }

        // Initialiser les variables en input
        DBSetVariableItemValue( hContextId, 0, &gsSvcWork.sParmWork.dwContextId, 0 );
        DBSetVariableItemValue( hPlazaId  , 0, &psLane->sId.dwPlaza            , 0 );
        DBSetVariableItemValue( hLaneId   , 0, &psLane->sId.dwLane             , 0 );
        DBSetVariableItemValue( hSeqNumber, 0, &psLane->dwLastTreated          , 0 );

        // Exécuter la requète
        NTSVCInfo( "DBSetLastTreatedNumber(), Exécution de la requête SetLastRequest" );
        if ( ! DBExecuteStatement( hStmt ) )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBSetLastTreatedNumber(), erreur %u, exécute SetLastRequest : %s", dwErr, szErr );
            break;
        }

        NTSVCInfo( "DBSetLastTreatedNumber(), Fin de la requête SetLastRequest" );
    }
    while ( FALSE );

    if ( ( hStmt != NULL ) && ( *phDbCnx != NULL ) )
        DBCloseStatement( hStmt );

    return dwErr;
}




PROTECTED DWORD DBGetLastTreatedNumber( DB_CNX ** phDbCnx, TODBSVC_LANE * psLane )
{
    char szErr[200] = "";    // Texte d'erreur
    DWORD dwErr = NO_ERROR;
    DWORD dwLast;
    DWORD * pdwLast;
    DB_VAR * hContextId;     // BIND : Handle du context
    DB_VAR * hPlazaId;       // BIND : Handle de l'id de gare
    DB_VAR * hLaneId;        // BIND : Handle de l'id de voie
    DB_VAR * hSeqNumber;     // BIND : Handle du numéro de séquence
    DB_STMT * hStmt;
 
    do
    {
        if ( *phDbCnx == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        // Préparer la requête
        NTSVCInfo( "DBGetLastTreatedNumber(), préparation de la requête GetLastRequest" );
        if ( ( hStmt = DBOpenStatement( *phDbCnx, gszDbGetLastRequest ) ) == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBGetLastTreatedNumber(), erreur %u, statement GetLastRequest : %s", dwErr, szErr );
            break;
        }

        // Associer les variables
        hContextId = DBBindPlaceHolderVariable(
                hStmt, 
                ":ContextId", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hContextId == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBSetLastTreatedNumber(), erreur %u, bind ContextId : %s", dwErr, szErr );
            break;
        }

        hPlazaId = DBBindPlaceHolderVariable(
                hStmt, 
                ":PlazaId", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hPlazaId == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBSetLastTreatedNumber(), erreur %u, bind PlazaId : %s", dwErr, szErr );
            break;
        }

        hLaneId = DBBindPlaceHolderVariable(
                hStmt, 
                ":LaneId", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hLaneId == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBSetLastTreatedNumber(), erreur %u, bind LaneId : %s", dwErr, szErr );
            break;
        }

        hSeqNumber = DBBindPlaceHolderVariable(
                hStmt, 
                ":SeqNumber", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hSeqNumber == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBSetLastTreatedNumber(), erreur %u, bind SeqNumber : %s", dwErr, szErr );
            break;
        }

        // Initialiser les variables en input
        DBSetVariableItemValue( hContextId, 0, &gsSvcWork.sParmWork.dwContextId, 0 );
        DBSetVariableItemValue( hPlazaId  , 0, &psLane->sId.dwPlaza            , 0 );
        DBSetVariableItemValue( hLaneId   , 0, &psLane->sId.dwLane             , 0 );

        // Exécuter la requète
        NTSVCInfo( "DBGetLastTreatedNumber(), Exécution de la requête GetLastRequest" );
        if ( ! DBExecuteStatement( hStmt ) )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBGetLastTreatedNumber(), erreur %u, exécute GetLastRequest : %s", dwErr, szErr );
            break;
        }

        NTSVCInfo( "DBGetLastTreatedNumber(), Fin de la requête GetLastRequest, début analyse" );

        // Récupérer le résultat
        pdwLast = DBGetVariableItemValue( hSeqNumber, 0, &dwLast, NULL );
        if ( pdwLast == &dwLast )
            psLane->dwLastTreated = dwLast;
        else if ( pdwLast == DB_VALUE_NULL )
            psLane->dwLastTreated = 0;
        else
        {
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            NTSVCInfo( "DBGetLastTreatedNumber(), impossible de déterminer le dernier numéro traité" );
            dwErr = ERROR_INVALID_DATA;
            break;
        }
        NTSVCInfo( "DBGetLastTreatedNumber(), Fin analyse" );
    }
    while ( FALSE );

    if ( ( hStmt != NULL ) && ( *phDbCnx != NULL ) )
        DBCloseStatement( hStmt );

    return dwErr;
}






PROTECTED DWORD DBInsertMessage( DB_CNX ** phDbCnx, char * pcMsg, DWORD dwSize )
{
    char szErr[200] = "";    // Texte d'erreur
    DWORD dwErr = NO_ERROR;
    DWORD dwNbMsg;
    DB_VAR * hMsg;            // BIND : Message
    DB_VAR * hNbMsg;          // BIND : Nombre de messages
    DB_STMT * hStmt;
 
    do
    {
        if ( *phDbCnx == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        // Préparer la requête
        NTSVCInfo( "DBInsertMessage(), préparation de la requête LoadMesRequest" );
        hStmt = DBOpenStatement( *phDbCnx, gszDbLoadMesRequest );
        if ( hStmt == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_OPEN", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBOpenStatement:LoadMesRequest): %s\n", szErr );
            else
                NTSVCInfo( "DBInsertMessage(), erreur %u, Statement LoadMesRequest : %s", dwErr, szErr );
            break;
        }

        // Associer les variables, mettre à jour leurs contenus
        hNbMsg = DBBindPlaceHolderVariable(
                hStmt, 
                ":NbMsg", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
        if ( hNbMsg == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_BIND", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBBindPlaceHolderVariable:LoadMesRequest): %s\n", szErr );
            else
                NTSVCInfo( "DBInsertMessage(), erreur %u, Bind NbMsg : %s", dwErr, szErr );
            break;
        }

        hMsg = DBBindPlaceHolderVariable(
                hStmt, 
                ":Msg", 
                DB_TYPE_STR, 
                dwSize, 
                1 );
        if ( hMsg == NULL )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_BIND", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBBindPlaceHolderVariable:LoadMesRequest): %s\n", szErr );
            else
                NTSVCInfo( "DBInsertMessage(), erreur %u, Bind Msg : %s", dwErr, szErr );
            break;
        }

        dwNbMsg = 1;
        DBSetVariableItemValue( hNbMsg, 0, &dwNbMsg, 0 );
        DBSetVariableItemValue( hMsg, 0, pcMsg, dwSize );

        // Exécuter la requète
        NTSVCInfo( "DBInsertMessage(), Exécution de la requête LoadMesRequest" );
        if ( ! DBExecuteStatement( hStmt ) )
        {
            DBGetLastError( *phDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            DBDisconnect( *phDbCnx );
            *phDbCnx = NULL;
            SVC_ERR_S( dwErr, "ERR_DB_EXEC", szErr );
            if ( gsSvcWork.bIsCommand ) 
                printf( "DATABASE ERROR (DBExecuteStatement:LoadMesRequest): %s\n", szErr );
            else
                NTSVCInfo( "DBInsertMessage(), erreur %u, Execute LoadMesRequest : %s", dwErr, szErr );
            break;
        }
        NTSVCInfo( "DBInsertMessage(), Fin de la requête LoadMesRequest" );
    }
    while ( FALSE );

    if ( ( hStmt != NULL ) && ( *phDbCnx != NULL ) )
        DBCloseStatement( hStmt );

    return dwErr;
}





PRIVATE DWORD DBGetLanesFile()
{
    FILE * pfFile;
    DWORD dwLine;
    DWORD dwCount;
    TODBSVC_LANE sLane;
    BOOL bStarted;
    BOOL bEnded;
    char szLine[1024];
	errno_t err;


    NTSVCInfo( "DBGetLanesFile(), Lecture de la configuration gare dans le fichier %s", gsSvcWork.sParmWork.szLanesFile );

    err = fopen_s( &pfFile, gsSvcWork.sParmWork.szLanesFile, "rt" );
    if ( err != 0 )
    {
        NTSVCInfo( "DBGetLanesFile(), erreur %u, fichier %s absent", ERROR_FILE_NOT_FOUND, gsSvcWork.sParmWork.szLanesFile );
        return ERROR_FILE_NOT_FOUND;
    }

    ColLock( gsSvcWork.hLanes );

    // Mettre à zéro la table des nouvelles voies
    ColClear( gsSvcWork.hLanes );

    dwLine = 0;
    dwCount = 0;
    bStarted = FALSE;
    bEnded;
    while ( fgets( szLine, sizeof(szLine), pfFile ) != NULL )
    {
        dwLine ++;
        DBTrim( szLine );
        if ( szLine[0] == '\0' ) continue;
        if ( szLine[0] == ';' ) continue;

        if ( ! bStarted )
        {
            if ( _stricmp( szLine, "BEGIN LANESLIST" ) != 0 )
            {
                ColClear( gsSvcWork.hLanes );
                ColUnlock( gsSvcWork.hLanes );
                fclose( pfFile );
                SVC_ERR( ERROR_INVALID_DATA, "ERR_FILE_FORMAT_HEADER" );
                return ERROR_INVALID_DATA;
            }
            bStarted = TRUE;
            continue;
        }

        if ( _stricmp( szLine, "END LANESLIST" ) == 0 )
        {
            bEnded = TRUE;
            break;
        }

        ZeroMemory( &sLane, sizeof(sLane) );
        if ( sscanf_s( szLine, "%u|%u", &sLane.sId.dwPlaza, &sLane.sId.dwLane ) != 2 )
        {
            ColClear( gsSvcWork.hLanes );
            ColUnlock( gsSvcWork.hLanes );
            fclose( pfFile );
            NTSVCInfo( "DBGetLanesFile(), erreur %u, données non valides dans %s ligne %u", ERROR_INVALID_DATA, gsSvcWork.sParmWork.szLanesFile, dwLine );
            return ERROR_INVALID_DATA;
        }

        if ( ColItemAdd( gsSvcWork.hLanes, &sLane.sId, &sLane, sizeof(sLane ) ) == NULL )
        {
            ColClear( gsSvcWork.hLanes );
            ColUnlock( gsSvcWork.hLanes );
            fclose( pfFile );
            SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_COL_ADD" );
            return ERROR_INVALID_DATA;
        }
        else
        {
            dwCount ++;
            NTSVCInfo( "DBGetLanesFile(), PZ%04u-LN%04u présente",
                              sLane.sId.dwPlaza, 
                              sLane.sId.dwLane );
        }
    }
    ColUnlock( gsSvcWork.hLanes );
    fclose( pfFile );

    if ( ! bEnded )
    {
        SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_FILE_FORMAT_FOOTER" );
        return ERROR_INVALID_DATA;
    }

    NTSVCInfo( "DBGetLanesFile(), %u voie sont enregistrées dans ce système", dwCount );

    return NO_ERROR;
}



PRIVATE DWORD DBSaveLanesFile()
{
    FILE * pfFile;
    HCOLLECTIONITEM hItem;
    TODBSVC_LANE * psLane;
	errno_t err;

    NTSVCInfo( "DBSaveLanesFile(), Ecriture de la configuration gare dans le fichier %s", gsSvcWork.sParmWork.szLanesFile );

    err = fopen_s( &pfFile, gsSvcWork.sParmWork.szLanesFile, "w+t" );
    if ( pfFile == NULL )
    {
        NTSVCInfo( "DBSaveLanesFile(), erreur %u, impossible de créer le fichier %s", ERROR_INVALID_DATA, gsSvcWork.sParmWork.szLanesFile );
        return ERROR_INVALID_DATA;
    }

    ColLock( gsSvcWork.hLanes );

    fprintf( 
        pfFile, 
        "; ********************************\n"
        "; *    LANES DEFINITION FILE     *\n"
        "; * LINE FORMAT : PlazaId|LaneId *\n"
        "; ********************************\n"
        "BEGIN LANESLIST\n" );

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hLanes, &hItem ) )
    {
        psLane = ColItemData( hItem );
        fprintf( pfFile, "%u|%u\n", psLane->sId.dwPlaza, psLane->sId.dwLane );
    }
    fprintf( 
        pfFile, 
        "END LANESLIST\n"
        "; ********************************\n"
        "; *       END OF THE FILE        *\n"
        "; ********************************\n" );

    ColUnlock( gsSvcWork.hLanes );
    fclose( pfFile );

    return NO_ERROR;
}


PRIVATE char * DBTrim( char * pcStr )
{
    char * pcSrc = pcStr;
    char * pcDst = pcStr;
    char * pcStop = NULL;

    while ( (*pcSrc > '\0') && (*pcSrc <= ' ') ) pcSrc ++;
    while ( *pcSrc != '\0' )
    {
        *pcDst = *pcSrc;
        if ( ( pcStop == NULL ) && ( *pcDst <= ' ' ) && (*pcDst >= '\0') )
            pcStop = pcDst;
        else if ( (*pcSrc < '\0') || (*pcSrc > ' ') )
            pcStop = NULL;
        pcDst ++;
        pcSrc ++;
    }
    if ( pcStop != NULL ) 
        *pcStop = '\0';
    else
        *pcDst = '\0';
    return pcStr;
}



