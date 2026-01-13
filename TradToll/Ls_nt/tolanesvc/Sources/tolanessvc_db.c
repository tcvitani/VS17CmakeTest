/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : tolanessvc
 * FILE       : tolanessvc_db.c
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

#include <acom.h>
#include <ntsvc.h>
#include <dbif.h>
#include <csr_list.h>
#include <col.h>
#include <reg.h>

#include <tolanessvc_glob.h>
#include <tolanessvc_text.h>
#include <tolanessvc_lane.h>

#define LOC_DEF
#include <tolanessvc_db.h>
#undef LOC_DEF

#include <memclass.h>


#define DB_DEFAULT_LANE_LIST \
       "BEGIN IF :ContextId <> 0 THEN NULL; END IF; GEN_DEF.Get_Lane_LS( :PlazaId, :PlazaName, :LaneId, :LaneName); :TotalCount := 0; END;"

#define DB_DEFAULT_GET_REF \
        "BEGIN IF :ContextId <> 0 THEN  NULL; END IF; IF GEN_FILE_LANE.Get_Ref_Lane( :PlazaId, :LaneId, :FileName ) <> 0 THEN  :FileName := ''; END IF; END;"


PRIVATE DWORD DBGetLanesFile();
PRIVATE DWORD DBSaveLanesFile();
PRIVATE BOOL DBIsValidReference( char * pcFileName );
PRIVATE char * DBTrim( char * pcStr );


PRIVATE char gszDbLaneListRequest[8000]  = "";
PRIVATE char gszDbGetRefRequest[8000]    = "";


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
            TOLANESSVC_REG_ROOT, \
            TOLANESSVC_REG_KEY_REQ, \
            #var, \
            gszDb##var, \
            &dwLen, \
            def ); \
        if ( dwErr != NO_ERROR ) \
            return dwErr;

    LOAD_REQ( LaneListRequest , DB_DEFAULT_LANE_LIST  )
    LOAD_REQ( GetRefRequest   , DB_DEFAULT_GET_REF  )

    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void DBCleanup( )
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Si on est encore connecté à la base, on la déconnecte
 * --------------------------------------------------------------------
 */
PROTECTED void DBCleanup( )
{
    if ( gsSvcWork.hDbCnx != NULL )
    {
        NTSVCInfo( "DBCleanup(), déconnection de la base" );
        DBDisconnect( gsSvcWork.hDbCnx );
        gsSvcWork.hDbCnx = NULL;
    }
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
PROTECTED DWORD DBGetLanes( )
{
    DWORD dwErr = NO_ERROR;  // Code d'erreur à retourner
    DWORD dwReqCount;
    DWORD dwCount;           // Stocker la taille des tables renvoyées
    DWORD dwIndex;           // Index pour scanner les éléments des tables
    DWORD dwLen;             // Taille de données
    DB_VAR * hContextId;     // BIND : Handle du context
    DB_VAR * hPlazaId;       // BIND : Handle du tableau des id de gare
    DB_VAR * hPlazaName;     // BIND : Handle du tableau des noms de gare
    DB_VAR * hLaneId;        // BIND : Handle du tableau des id de voie
    DB_VAR * hLaneName;      // BIND : Handle du tableau des noms de voie
    DB_VAR * hFileName;      // BIND : Handle du scalaire du nom du fichier de référence
    DB_VAR * hTotalCount;    // BIND : Handle du nombre d'élement
    DB_STMT * hStmt;         // Handle de la requete
    void * pvResult;         // Pour récuperer un pointeur sur la valeur d'un bind
    char szErr[200] = "";    // Texte d'erreur
    char szPlaza[TOLANESSVC_MAX_PLAZA_NAME] = ""; // Nom de gare
    char szLane[TOLANESSVC_MAX_PLAZA_NAME] = "";  // Nom de voie
    char szRefFile[MAX_PATH];

    if ( gsSvcWork.sParmWork.dwLanesFileOnly == 0 )
    {
        // Boucle do while() utilisée pour la commodité du break. En faite, la condition
        // de bouclage est FALSE, on ne passe donc qu'une fois dedans.
        do 
        {
            // Si pas connecté, on essaye de rétablir la connexion
            if ( gsSvcWork.hDbCnx == NULL )
            {
                NTSVCInfo( "DBGetLanes(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr );
                gsSvcWork.hDbCnx  = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
                gsSvcWork.bDatabaseMissing = ( gsSvcWork.hDbCnx == NULL );
            }

            // Si toujours pas connecté, on ne va pas plue long
            if ( gsSvcWork.hDbCnx == NULL )
            {
                dwErr = ERROR_PIPE_NOT_CONNECTED;
                NTSVCInfo( "DBGetLanes(), erreur %u, connexion à la base impossible", dwErr );
                break;
            }

            // REQUETE : OBTENIR LA LISTE DES VOIE POUR CE SYSTEME

            NTSVCInfo( "DBGetLanes(), préparation de la requête LaneListRequest" );

            // Préparer la requête
            hStmt = DBOpenStatement( gsSvcWork.hDbCnx, gszDbLaneListRequest );
            if ( hStmt == NULL  )
            {
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
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
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
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
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind PlazaId : %s", dwErr, szErr );
                DBCloseStatement( hStmt );
                break;
            }

            hPlazaName = DBBindPlaceHolderVariable( 
                    hStmt, 
                    ":PlazaName", 
                    DB_TYPE_STR, 
                    TOLANESSVC_MAX_PLAZA_NAME, 
                    gsSvcWork.sParmWork.dwMaxLanes );
            if ( hPlazaName == NULL )
            {
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind PlazaName : %s", dwErr, szErr );
                DBCloseStatement( hStmt );
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
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind LaneId : %s", dwErr, szErr );
                DBCloseStatement( hStmt );
                break;
            }

            hLaneName = DBBindPlaceHolderVariable( 
                    hStmt, 
                    ":LaneName", DB_TYPE_STR,
                    TOLANESSVC_MAX_LANE_NAME, 
                    gsSvcWork.sParmWork.dwMaxLanes );
            if ( hLaneName == NULL )
            {
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind LaneName : %s", dwErr, szErr );
                DBCloseStatement( hStmt );
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
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind TotalCount : %s", dwErr, szErr );
                DBCloseStatement( hStmt );
                break;
            }

            // Initialiser les variables en input
            DBSetVariableItemValue( hContextId, 0, &gsSvcWork.sParmWork.dwContextId, 0 );

            // Exécuter la requète
            if ( ! DBExecuteStatement( hStmt ) )
            {
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, exécute LaneListRequest : %s", dwErr, szErr );
                DBCloseStatement( hStmt );
                break;
            }

            // Récupérer la taille des tableaux renvoyée par la base
            if ( DBGetVariableItemValue( hTotalCount, 0, &dwReqCount, NULL ) != &dwReqCount )
            {
                dwErr = ERROR_INVALID_DATA;
                NTSVCInfo( "DBGetLanes(), erreur %u, nombre d'éléments de table incohérent", dwErr );
                DBCloseStatement( hStmt );
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
                NTSVCInfo( "DBGetLanes(), erreur %u, nombre d'éléments de table incohérent ou trop grand", dwErr );
                DBCloseStatement( hStmt );
                break;
            }

            // Mettre à zéro la table des nouvelles voies
            ZeroMemory( gsSvcWork.psListNew, gsSvcWork.sParmWork.dwMaxLanes * sizeof(*gsSvcWork.psListNew) );

            // Remplir la table avec les nouvelles valeurs
            for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
            {
                // Arrêter de parcourir si valeur non valide
                pvResult = DBGetVariableItemValue( hPlazaId, dwIndex, &gsSvcWork.psListNew[dwIndex].dwPlaza, NULL );
                if ( pvResult != &gsSvcWork.psListNew[dwIndex].dwPlaza )
                {
                    // Ajuster la le nombre des éléments lus
                    dwCount = dwIndex;
                    break;
                }
                pvResult = DBGetVariableItemValue( hLaneId, dwIndex, &gsSvcWork.psListNew[dwIndex].dwLane, NULL );
                if ( pvResult != &gsSvcWork.psListNew[dwIndex].dwLane )
                {
                    // Ajuster la le nombre des éléments lus
                    dwCount = dwIndex;
                    break;
                }
                pvResult = DBGetVariableItemValue( hPlazaName, dwIndex, szPlaza, NULL );
                if ( pvResult != szPlaza ) szPlaza[0] = '\0';
                pvResult = DBGetVariableItemValue( hLaneName, dwIndex, szLane, NULL );
                if ( pvResult != szLane ) szLane[0] = '\0';
                gsSvcWork.psListNew[dwIndex].bBusy = TRUE;
                NTSVCInfo( "DBGetLanes(), PZ%04u-LN%04u présente ([%s]-[%s])",
                                  gsSvcWork.psListNew[dwIndex].dwPlaza, 
                                  gsSvcWork.psListNew[dwIndex].dwLane,
                                  szPlaza,
                                  szLane );
            }
            DBCloseStatement( hStmt );
            NTSVCInfo( "DBGetLanes(), %u voie sont enregistrées dans ce système", dwCount );

            // REQUETE : OBTENIR LE NOM DU FICHIER DE REFERENCE DE CHAQUE VOIE

            NTSVCInfo( "DBGetLanes(), préparation de la requête GetRefRequest" );

            // Préparer la requete
            hStmt = DBOpenStatement( gsSvcWork.hDbCnx, gszDbGetRefRequest );
            if ( hStmt == NULL  )
            {
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, statement GetRefRequest : %s", dwErr, szErr );
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
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind ContextId : %s", dwErr, szErr );
                DBCloseStatement( hStmt );
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
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind PlazaId : %s", dwErr, szErr );
                DBCloseStatement( hStmt );
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
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind LaneId : %s", dwErr, szErr );
                DBCloseStatement( hStmt );
                break;
            }

            hFileName = DBBindPlaceHolderVariable( 
                    hStmt, 
                    ":FileName",
                    DB_TYPE_STR, 
                    sizeof(szRefFile), 
                    0 );
            if ( hFileName == NULL )
            {
                DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                NTSVCInfo( "DBGetLanes(), erreur %u, bind FileName : %s", dwErr, szErr );
                DBCloseStatement( hStmt );
                break;
            }

            // Pour toutes les voies
            for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
            {
                DBClearVariables( hStmt );
                DBSetVariableItemValue( hContextId, 0, &gsSvcWork.sParmWork.dwContextId, 0 );
                DBSetVariableItemValue( hPlazaId, 0, &gsSvcWork.psListNew[dwIndex].dwPlaza, 0 );
                DBSetVariableItemValue( hLaneId, 0, &gsSvcWork.psListNew[dwIndex].dwLane, 0 );

                // Exécuter la requète
                if ( ! DBExecuteStatement( hStmt ) )
                {
                    DBGetLastError( gsSvcWork.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
                    NTSVCInfo( "DBGetLanes(), erreur %u, exécute GetRefRequest (PZ%04u-LN%04u) : %s", dwErr, gsSvcWork.psListNew[dwIndex].dwPlaza, gsSvcWork.psListNew[dwIndex].dwLane, szErr );
                    break;
                }

                dwLen = sizeof( szRefFile );
                pvResult = DBGetVariableItemValue( 
                        hFileName, 
                        0, 
                        szRefFile, 
                        &dwLen );
                if ( pvResult != szRefFile )
                {
                    dwErr = ERROR_INVALID_DATA;
                    break;
                }
                if ( ! DBIsValidReference( szRefFile ) )
                {
                    NTSVCInfo( "DBGetLanes(), le fichier de référence '%s' (PZ%04u-LN%04u) fourni par la procédure stockée n'est pas valide", szRefFile, gsSvcWork.psListNew[dwIndex].dwPlaza, gsSvcWork.psListNew[dwIndex].dwLane, szErr );
                    strcpy_s( szRefFile, sizeof(szRefFile), "" );
                }

                strcpy_s( gsSvcWork.psListNew[dwIndex].szRefFile, MAX_PATH, szRefFile );

                NTSVCInfo( 
                        "DBGetLanes(), référence PZ%04u-LN%04u : REF=[%s]", 
                        gsSvcWork.psListNew[dwIndex].dwPlaza,
                        gsSvcWork.psListNew[dwIndex].dwLane,
                        gsSvcWork.psListNew[dwIndex].szRefFile );
            }

            DBCloseStatement( hStmt );

            if ( dwErr != NO_ERROR )
                break;
        }
        while ( FALSE );
    
        if ( ( gsSvcWork.hDbCnx != NULL ) && 
             ( ( gsSvcWork.sParmWork.dwPermanentDB == 0 ) ||
               ( dwErr != NO_ERROR                      )
             )
           )
        {
            NTSVCInfo( "DBGetLanes(), déconnection de la base" );
            DBDisconnect( gsSvcWork.hDbCnx );
            gsSvcWork.hDbCnx = NULL;
        }

        if ( dwErr != NO_ERROR )
        {
            ZeroMemory( gsSvcWork.psListNew, gsSvcWork.sParmWork.dwMaxLanes * sizeof(*gsSvcWork.psListNew) );
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
    
    if ( dwErr == NO_ERROR )
    {
        // Pas d'erreur, on fait la synthèse de l'ancienne liste avec la nouvelle
        EnterCriticalSection( &gsSvcWork.sCritical );
        LaneListMerge( gsSvcWork.psList, gsSvcWork.psListNew );
        LeaveCriticalSection( &gsSvcWork.sCritical );
    }

    return dwErr;
}




PRIVATE DWORD DBGetLanesFile()
{
    FILE * pfFile;
    DWORD dwLine;
    DWORD dwCount;
    TOLANESSVC_LANE * psLane;
    BOOL bStarted;
    BOOL bEnded;
	errno_t err;
    char szLine[MAX_PATH+4];


    NTSVCInfo( "DBGetLanesFile(), Lecture de la configuration gare dans le fichier %s", gsSvcWork.sParmWork.szLanesFile );

    ZeroMemory( gsSvcWork.psListNew, gsSvcWork.sParmWork.dwMaxLanes * sizeof(*gsSvcWork.psListNew) );

    err = fopen_s( &pfFile, gsSvcWork.sParmWork.szLanesFile, "rt" );
    if ( err != 0 )
    {
        NTSVCInfo( "DBGetLanesFile(), erreur %u, fichier %s absent", ERROR_FILE_NOT_FOUND, gsSvcWork.sParmWork.szLanesFile );
        return ERROR_FILE_NOT_FOUND;
    }

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
                fclose( pfFile );
                ZeroMemory( gsSvcWork.psListNew, gsSvcWork.sParmWork.dwMaxLanes * sizeof(*gsSvcWork.psListNew) );
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

        if ( dwCount >= gsSvcWork.sParmWork.dwMaxLanes )
        {
            fclose( pfFile );
            ZeroMemory( gsSvcWork.psListNew, gsSvcWork.sParmWork.dwMaxLanes * sizeof(*gsSvcWork.psListNew) );
            SVC_ERR( ERROR_INVALID_DATA, "ERR_FILE_TOO_MANY_LANES" );
            return ERROR_INVALID_DATA;
        }

        psLane = &gsSvcWork.psListNew[dwCount];

        if ( sscanf_s( szLine, "%u|%u|%[^|]", &psLane->dwPlaza, &psLane->dwLane, psLane->szRefFile, sizeof(szLine) ) != 3 )
        {
            fclose( pfFile );
            ZeroMemory( gsSvcWork.psListNew, gsSvcWork.sParmWork.dwMaxLanes * sizeof(*gsSvcWork.psListNew) );
            NTSVCInfo( "DBGetLanesFile(), erreur %u, données non valides dans %s ligne %u", ERROR_INVALID_DATA, gsSvcWork.sParmWork.szLanesFile, dwLine );
            return ERROR_INVALID_DATA;
        }
        if ( strcmp( psLane->szRefFile, "." ) == 0 ) psLane->szRefFile[0] = '\0';
        psLane->bBusy = TRUE;
        NTSVCInfo( "DBGetLanes(), référence PZ%04u-LN%04u : REF=[%s]", 
                          psLane->dwPlaza, 
                          psLane->dwLane,
                          psLane->szRefFile);
        dwCount ++;
    }
    fclose( pfFile );

    if ( ! bEnded )
    {
        ZeroMemory( gsSvcWork.psListNew, gsSvcWork.sParmWork.dwMaxLanes * sizeof(*gsSvcWork.psListNew) );
        SVC_ERR( ERROR_NOT_ENOUGH_MEMORY, "ERR_FILE_FORMAT_FOOTER" );
        return ERROR_INVALID_DATA;
    }

    NTSVCInfo( "DBGetLanesFile(), %u voie sont enregistrées dans ce système", dwCount );
    return NO_ERROR;
}



PRIVATE DWORD DBSaveLanesFile()
{
    FILE * pfFile;
    TOLANESSVC_LANE * psLane;
    DWORD dwIndex;
	errno_t err;

    NTSVCInfo( "DBSaveLanesFile(), Ecriture de la configuration gare dans le fichier %s", gsSvcWork.sParmWork.szLanesFile );

    err = fopen_s( &pfFile, gsSvcWork.sParmWork.szLanesFile, "w+t" );
    if ( err != 0 )
    {
        NTSVCInfo( "DBSaveLanesFile(), erreur %u, impossible de créer le fichier %s", ERROR_INVALID_DATA, gsSvcWork.sParmWork.szLanesFile );
        return ERROR_INVALID_DATA;
    }

    fprintf( 
        pfFile, 
        "; ****************************************\n"
        "; *        LANES DEFINITION FILE         *\n"
        "; * LINE FORMAT : PlazaId|LaneId|RefFile *\n"
        "; ****************************************\n"
        "BEGIN LANESLIST\n" );

    for ( dwIndex = 0 ; dwIndex < gsSvcWork.sParmWork.dwMaxLanes ; dwIndex ++ )
    {
        psLane = &gsSvcWork.psListNew[dwIndex];
        if ( psLane->bBusy ) fprintf( pfFile, "%u|%u|%s\n", psLane->dwPlaza, psLane->dwLane, psLane->szRefFile[0] == '\0' ? "." : psLane->szRefFile );
    }
    fprintf( 
        pfFile, 
        "END LANESLIST\n"
        "; ****************************************\n"
        "; *           END OF THE FILE            *\n"
        "; ****************************************\n" );

    fclose( pfFile );

    return NO_ERROR;
}



PRIVATE BOOL DBIsValidReference( char * pcFileName )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwLine;
    FILE * pfFile = NULL;
    HANDLE hFile;
    char * pcReference;
    char * pcTerminator;
    char szPath[MAX_PATH*2];
    char szPathRef[MAX_PATH*2];
    char szLine[MAX_PATH*2];
	errno_t err;

    __try
    {
        if ( pcFileName[0] == '\0' )                                                              
        {
            NTSVCInfo( "DBIsValidReference(), le nom du fichier de référence est vide" );
            __leave;
        }

        _snprintf_s( szPath, _countof(szPath), sizeof(szPath), "%s\\%s", gsSvcWork.sParmWork.szReferenceDir, pcFileName );
        szPath[sizeof(szPath)-1] = '\0';

        err = fopen_s( &pfFile, szPath, "rt" );
        if ( err != 0 )                               
        { 
            NTSVCInfo( "DBIsValidReference(), le nom du fichier de référence '%s' n'a pas été trouvé sous le chemin '%s'", pcFileName, gsSvcWork.sParmWork.szReferenceDir );
            dwErr = ERROR_FILE_NOT_FOUND;       
            __leave; 
        }

        dwLine = 0;
        while ( fgets( szLine, sizeof(szLine), pfFile ) != NULL )
        {
            dwLine ++;
            DBTrim( szLine );

            if ( szLine[0] == '\0' ) continue;
            if ( szLine[0] == ';' ) continue;
            if ( szLine[0] == '#' ) continue;

            pcReference = strchr( szLine, '=' );
            if ( pcReference == NULL )                      
            {
                NTSVCInfo( "DBIsValidReference(), erreur à la ligne %u du fichier de référence '%s'", dwLine, pcFileName );
                dwErr = ERROR_INVALID_DATA;         
                __leave; 
            }
            DBTrim( ++pcReference );

            pcTerminator = strchr( pcReference, ' ' );
            if ( pcTerminator != NULL ) (*pcTerminator) = '\0';
            DBTrim( pcReference );

            _snprintf_s( szPathRef, _countof(szPathRef), sizeof(szPathRef), "%s\\%s", gsSvcWork.sParmWork.szReferenceDir, pcReference );
            szPathRef[sizeof(szPathRef)-1] = '\0';

            // Tenter une ouverture en laissant l'accés en lecture, mais en vérouillant
            // l'accés en écriture (si un process écrit dedans, le fichier n'est pas prét
            // et l'ouverture echouera).
            hFile = CreateFile( 
                szPathRef, 
                GENERIC_READ, 
                FILE_SHARE_READ, 
                NULL, 
                OPEN_EXISTING, 
                FILE_ATTRIBUTE_NORMAL,
                NULL );
            if ( hFile == INVALID_HANDLE_VALUE )            
            { 
                dwErr = GetLastError();
                if ( dwErr == ERROR_FILE_NOT_FOUND )
                    NTSVCInfo( "DBIsValidReference(), le fichier '%s', référence par '%s' n'existe pas", pcReference, pcFileName );
                else
                    NTSVCInfo( "DBIsValidReference(), le fichier '%s', référence par '%s' ne peut être ouvert à cause d'une erreur système %u", pcReference, pcFileName, dwErr );
                __leave; 
            }

            CloseHandle( hFile );
        }
    }
    __finally
    {
        if ( pfFile != NULL ) 
            fclose( pfFile );
    }

	return ( dwErr == NO_ERROR );
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


